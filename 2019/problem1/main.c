#include <time.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

typedef enum
{
    ZERO = 0,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
} number_t;

#define MAX_LINE (32)

///////////////////////////////////////////////////////////////////////

#define CJSON_CDECL
#define CJSON_STDCALL

#define CJSON_PUBLIC(type) static type

/* CJSON Types: */
#define CJSON_INVALID (ZERO)
#define CJSON_FALSE  (ONE << ZERO)
#define CJSON_TRUE   (ONE << ONE)
#define CJSON_NULL   (ONE << TWO)
#define CJSON_NUMBER (ONE << THREE)
#define CJSON_STRING (ONE << FOUR)
#define CJSON_ARRAY  (ONE << FIVE)
#define CJSON_OBJECT (ONE << SIX)
#define CJSON_RAW    (ONE << SEVEN) /* raw json */
#define CJSON_MAX    (0xff)

#define CJSON_ISREFERENCE 256
#define CJSON_STRINGISCONST 512

/* The CJSON structure: */
typedef struct CJSON {
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct CJSON *next;
    struct CJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct CJSON *child;

    /* The type of the item, as above. */
    unsigned int type;

    /* The item's string, if type==CJSON_STRING  and type == CJSON_RAW */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use CJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==CJSON_NUMBER */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} CJSON;

typedef int CJSON_bool;

/* Limits how deeply nested arrays/objects can be before CJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* Render a CJSON entity to text for transfer/storage. */
CJSON_PUBLIC(char *) cJsonPrint(const CJSON *item);
/* Delete a CJSON entity and all subentities. */
CJSON_PUBLIC(void) cJsonDelete(CJSON *c);

/* These calls create a CJSON item of the appropriate type. */
CJSON_PUBLIC(CJSON *) cJsonCreateNumber(double num);
CJSON_PUBLIC(CJSON *) cJsonCreateString(const char *string);
/* raw json */
CJSON_PUBLIC(CJSON *) cJsonCreateArray(void);
CJSON_PUBLIC(CJSON *) cJsonCreateObject(void);

/* Append item to the specified array/object. */
CJSON_PUBLIC(void) cJsonAddItemToArray(CJSON *array, CJSON *item);
CJSON_PUBLIC(void) cJsonAddItemToObject(CJSON *object, const char *string,
        CJSON *item);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define CJSON_SETINTVALUE(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))

/* Macro for iterating over an array or object */
#define CJSON_ARRAYFOREACH(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* CJSON */
/* JSON parser in C. */
#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

/* define our own boolean type */
#define TRUE ((CJSON_bool)ONE)
#define FALSE ((CJSON_bool)ZERO)

typedef struct internal_hooks {
    void *(CJSON_CDECL *allocate)(size_t size);
    void (CJSON_CDECL *deallocate)(void *pointer);
    void *(CJSON_CDECL *reallocate)(void *pointer, size_t size);
} internal_hooks;

static internal_hooks global_hooks = { malloc, free, realloc };

static unsigned char* cJsonStrdup(const unsigned char* string, const internal_hooks * const hooks) {
    size_t length = ZERO;
    unsigned char *copy = NULL;

    if (string == NULL) {
        return NULL;
    }

    length = strlen((const char*) string) + sizeof("");
    copy = (unsigned char*) hooks->allocate(length);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

/* Internal constructor. */
static CJSON *cJsonNewItem(const internal_hooks * const hooks) {
    CJSON* node = (CJSON*) hooks->allocate(sizeof(CJSON));
    if (node) {
        memset(node, '\0', sizeof(CJSON));
    }

    return node;
}

/* Delete a CJSON structure. */
CJSON_PUBLIC(void) cJsonDelete(CJSON *item1) {
    CJSON *item = item1;
    CJSON *next = NULL;
    while (item != NULL) {
        next = item->next;
        if (!(item->type & CJSON_ISREFERENCE) && (item->child != NULL)) {
            cJsonDelete(item->child);
        }
        if (!(item->type & CJSON_ISREFERENCE) && (item->valuestring != NULL)) {
            global_hooks.deallocate(item->valuestring);
        }
        if (!(item->type & CJSON_STRINGISCONST) && (item->string != NULL)) {
            global_hooks.deallocate(item->string);
        }
        global_hooks.deallocate(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char getDecimalPoint(void) {
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char) lconv->decimalPoint[ZERO];
#else
    return '.';
#endif
}

typedef struct {
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth;
    internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with ONE) */
#define CAN_READ(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with ZERO) */
#define CAN_ACCESS_AT_INDEX(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define CANNOT_ACCESS_AT_INDEX(buffer, index) (!CAN_ACCESS_AT_INDEX(buffer, index))
/* get a pointer to the buffer at the position */
#define BUFFER_AT_OFFSET(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static CJSON_bool parseNumber(CJSON * const item, parse_buffer * const inputBuffer) {
    double number = ZERO;
    unsigned char *afterEnd = NULL;
    unsigned char numberCString[64];
    unsigned char decimalPoint = getDecimalPoint();
    size_t i = ZERO;

    if ((inputBuffer == NULL) || (inputBuffer->content == NULL)) {
        return FALSE;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = ZERO; (i < (sizeof(numberCString) - ONE)) && CAN_ACCESS_AT_INDEX(inputBuffer, i); i++) {
        switch (BUFFER_AT_OFFSET(inputBuffer)[i]) {
        case '0' ... '9':
        case '+':
        case '-':
        case 'e':
        case 'E':
            numberCString[i] = BUFFER_AT_OFFSET(inputBuffer)[i];
            break;

        case '.':
            numberCString[i] = decimalPoint;
            break;

        default:
            goto loop_end;
        }
    }
loop_end:
    numberCString[i] = '\0';

    number = strtod((const char*) numberCString, (char**) &afterEnd);
    if (numberCString == afterEnd) {
        return FALSE; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX) {
        item->valueint = INT_MAX;
    } else if (number <= (double) INT_MIN) {
        item->valueint = INT_MIN;
    } else {
        item->valueint = (int) number;
    }

    item->type = CJSON_NUMBER;

    inputBuffer->offset += (size_t)(afterEnd - numberCString);
    return TRUE;
}

typedef struct {
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth;
    CJSON_bool noalloc;
    CJSON_bool format;
    internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(printbuffer * const p, size_t neededLen) {
    unsigned char *newbuffer = NULL;
    size_t newsize = ZERO;
    size_t needed = neededLen;

    if ((p == NULL) || (p->buffer == NULL)) {
        return NULL;
    }

    if ((p->length > ZERO) && (p->offset >= p->length)) {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX) {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + ONE;
    if (needed <= p->length) {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / TWO)) {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            return NULL;
        }
    } else {
        newsize = needed * TWO;
    }

    if (p->hooks.reallocate != NULL) {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char*) p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL) {
            p->hooks.deallocate(p->buffer);
            p->length = ZERO;
            p->buffer = NULL;

            return NULL;
        }
    } else {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char*) p->hooks.allocate(newsize);
        if (!newbuffer) {
            p->hooks.deallocate(p->buffer);
            p->length = ZERO;
            p->buffer = NULL;

            return NULL;
        }
        if (newbuffer) {
            memcpy(newbuffer, p->buffer, p->offset + ONE);
        }
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return &newbuffer[p->offset];
}

/* calculate the new length of the string in a printbuffer and update the offset */
static void updateOffset(printbuffer * const buffer) {
    const unsigned char *bufferPointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL)) {
        return;
    }
    bufferPointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char*) bufferPointer);
}

/* Render the number nicely from the given item into a string. */
static CJSON_bool printNumber(const CJSON * const item,
        printbuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    double d = item->valuedouble;
    int length = ZERO;
    size_t i = ZERO;
    unsigned char numberBuffer[26];
    unsigned char decimalPoint = getDecimalPoint();

    if (outputBuffer == NULL) {
        return FALSE;
    }

    /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
    length = snprintf((char*) numberBuffer, sizeof(numberBuffer), "%1.15g", d);

    /* sprintf failed or buffer overrun occurred */
    if ((length < ZERO) || (length > (int) (sizeof(numberBuffer) - ONE))) {
        return FALSE;
    }

    /* reserve appropriate space in the output */
    outputPointer = ensure(outputBuffer, (size_t) length + sizeof(""));
    if (outputPointer == NULL) {
        return FALSE;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = ZERO; i < ((size_t) length); i++) {
        if (numberBuffer[i] == decimalPoint) {
            outputPointer[i] = '.';
        } else {
            outputPointer[i] = numberBuffer[i];
        }
    }
    outputPointer[i] = '\0';

    outputBuffer->offset += (size_t) length;

    return TRUE;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static CJSON_bool parseString(CJSON * const item,
        parse_buffer * const inputBuffer) {
    const unsigned char *inputPointer = BUFFER_AT_OFFSET(inputBuffer) + ONE;
    const unsigned char *inputEnd = BUFFER_AT_OFFSET(inputBuffer) + ONE;
    unsigned char *outputPointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (BUFFER_AT_OFFSET(inputBuffer)[ZERO] != '\"') {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocationLength = ZERO;
        size_t skippedBytes = ZERO;
        while (((size_t)(inputEnd - inputBuffer->content)
                < inputBuffer->length) && (*inputEnd != '\"')) {
            /* is escape sequence */
            if (inputEnd[ZERO] == '\\') {
                if ((size_t)(inputEnd + ONE - inputBuffer->content)
                        >= inputBuffer->length) {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skippedBytes++;
                inputEnd++;
            }
            inputEnd++;
        }
        if (((size_t)(inputEnd - inputBuffer->content) >= inputBuffer->length)
                || (*inputEnd != '\"')) {
            goto fail;
            /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocationLength = (size_t)(inputEnd - BUFFER_AT_OFFSET(inputBuffer))
                - skippedBytes;
        output = (unsigned char*) inputBuffer->hooks.allocate(
                allocationLength + sizeof(""));
        if (output == NULL) {
            goto fail;
            /* allocation failure */
        }
    }

    outputPointer = output;
    /* loop through the string literal */
    while (inputPointer < inputEnd) {
        if (*inputPointer != '\\') {
            *outputPointer++ = *inputPointer++;
        } else {
            unsigned char sequenceLength = TWO;
            if ((inputEnd - inputPointer) < ONE) {
                goto fail;
            }

            switch (inputPointer[ONE]) {
            case 'b':
                *outputPointer++ = '\b';
                break;
            case 'f':
                *outputPointer++ = '\f';
                break;
            case 'n':
                *outputPointer++ = '\n';
                break;
            case 'r':
                *outputPointer++ = '\r';
                break;
            case 't':
                *outputPointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
                *outputPointer++ = inputPointer[ONE];
                break;

            default:
                goto fail;
            }
            inputPointer += sequenceLength;
        }
    }

    /* zero terminate the output */
    *outputPointer = '\0';

    item->type = CJSON_STRING;
    item->valuestring = (char*) output;

    inputBuffer->offset = (size_t)(inputEnd - inputBuffer->content);
    inputBuffer->offset++;

    return TRUE;

fail:
    if (output != NULL) {
        inputBuffer->hooks.deallocate(output);
    }

    if (inputPointer != NULL) {
        inputBuffer->offset = (size_t)(inputPointer - inputBuffer->content);
    }

    return FALSE;
}

/* Render the cstring provided to an escaped version that can be printed. */
static CJSON_bool printStringPtr(const unsigned char * const input,
        printbuffer * const outputBuffer) {
    const unsigned char *inputPointer = NULL;
    unsigned char *output = NULL;
    unsigned char *outputPointer = NULL;
    size_t outputLength = ZERO;
    /* numbers of additional characters needed for escaping */
    size_t escapeCharacters = ZERO;

    if (outputBuffer == NULL) {
        return FALSE;
    }

    /* empty string */
    if (input == NULL) {
        output = ensure(outputBuffer, sizeof("\"\""));
        if (output == NULL) {
            return FALSE;
        }
        strncpy((char*) output, "\"\"", sizeof("\"\""));

        return TRUE;
    }

    /* set "flag" to ONE if something needs to be escaped */
    for (inputPointer = input; *inputPointer; inputPointer++) {
        switch (*inputPointer) {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            /* one character escape sequence */
            escapeCharacters++;
            break;
        default:
            if (*inputPointer < MAX_LINE) {
                /* UTF-16 escape sequence uXXXX */
                escapeCharacters += FIVE;
            }
            break;
        }
    }
    outputLength = (size_t)(inputPointer - input) + escapeCharacters;

    output = ensure(outputBuffer, outputLength + sizeof("\"\""));
    if (output == NULL) {
        return FALSE;
    }

    /* no characters have to be escaped */
    if (escapeCharacters == ZERO) {
        output[ZERO] = '\"';
        memcpy(output + ONE, input, outputLength);
        output[outputLength + ONE] = '\"';
        output[outputLength + TWO] = '\0';

        return TRUE;
    }

    output[ZERO] = '\"';
    outputPointer = output + ONE;
    /* copy the string */
    for (inputPointer = input; *inputPointer != '\0';
            (void) inputPointer++, outputPointer++) {
        if ((*inputPointer > MAX_LINE - ONE) && (*inputPointer != '\"')
                && (*inputPointer != '\\')) {
            /* normal character, copy */
            *outputPointer = *inputPointer;
        } else {
            /* character needs to be escaped */
            *outputPointer++ = '\\';
            switch (*inputPointer) {
            case '\\':
                *outputPointer = '\\';
                break;
            case '\"':
                *outputPointer = '\"';
                break;
            case '\b':
                *outputPointer = 'b';
                break;
            case '\f':
                *outputPointer = 'f';
                break;
            case '\n':
                *outputPointer = 'n';
                break;
            case '\r':
                *outputPointer = 'r';
                break;
            case '\t':
                *outputPointer = 't';
                break;
            default:
                /* escape and print as unicode codepoint */
                snprintf((char*) outputPointer, outputLength + sizeof("\"\""), "u%04x", *inputPointer);
                outputPointer += FOUR;
                break;
            }
        }
    }
    output[outputLength + ONE] = '\"';
    output[outputLength + TWO] = '\0';

    return TRUE;
}

/* Invoke printStringPtr (which is useful) on an item. */
static CJSON_bool printString(const CJSON * const item, printbuffer * const p) {
    return printStringPtr((unsigned char*) item->valuestring, p);
}

/* Predeclare these prototypes. */
static CJSON_bool parseValue(CJSON * const item,
        parse_buffer * const inputBuffer);
static CJSON_bool printValue(const CJSON * const item,
        printbuffer * const outputBuffer);
static CJSON_bool parseArray(CJSON * const item,
        parse_buffer * const inputBuffer);
static CJSON_bool printArray(const CJSON * const item,
        printbuffer * const outputBuffer);
static CJSON_bool parseObject(CJSON * const item,
        parse_buffer * const inputBuffer);
static CJSON_bool printObject(const CJSON * const item,
        printbuffer * const outputBuffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *bufferSkipWhitespace(parse_buffer * const buffer) {
    if ((buffer == NULL) || (buffer->content == NULL)) {
        return NULL;
    }

    while (CAN_ACCESS_AT_INDEX(buffer, ZERO) && (BUFFER_AT_OFFSET(buffer)[ZERO] <= MAX_LINE)) {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length) {
        buffer->offset--;
    }

    return buffer;
}

#define CJSON_MIN(a, b) ((a < b) ? a : b)

static unsigned char *print(const CJSON * const item, CJSON_bool format,
        const internal_hooks * const hooks) {
    static const size_t defaultBufferSize = 256;
    printbuffer buffer[ONE];
    unsigned char *printed = NULL;

    memset(buffer, ZERO, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char*) hooks->allocate(defaultBufferSize);
    buffer->length = defaultBufferSize;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL) {
        goto fail;
    }

    /* print the value */
    if (!printValue(item, buffer)) {
        goto fail;
    }
    updateOffset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL) {
        printed = (unsigned char*) hooks->reallocate(buffer->buffer,
                buffer->offset + ONE);
        if (printed == NULL) {
            goto fail;
        }
        buffer->buffer = NULL;
    } else {
        printed = (unsigned char*) hooks->allocate(buffer->offset + ONE);
        if (printed == NULL) {
            goto fail;
        }
        memcpy(printed, buffer->buffer,
                CJSON_MIN(buffer->length, buffer->offset + ONE));
        printed[buffer->offset] = '\0';

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
    }

    return printed;

    fail: if (buffer->buffer != NULL) {
        hooks->deallocate(buffer->buffer);
    }

    if (printed != NULL) {
        hooks->deallocate(printed);
    }

    return NULL;
}

/* Render a CJSON item/entity/structure to text. */
CJSON_PUBLIC(char *) cJsonPrint(const CJSON *item)
{
    return (char*)print(item, TRUE, &global_hooks);
}

/* Parser core - when encountering text, process appropriately. */
static CJSON_bool parseValue(CJSON * const item,
        parse_buffer * const inputBuffer) {
    if ((inputBuffer == NULL) || (inputBuffer->content == NULL)) {
        return FALSE;
    }

    /* parse the different types of values */
    /* null */
    if (CAN_READ(inputBuffer, FOUR)
            && (strncmp((const char*) BUFFER_AT_OFFSET(inputBuffer), "null", FOUR)
                    == ZERO)) {
        item->type = CJSON_NULL;
        inputBuffer->offset += FOUR;
        return TRUE;
    }
    /* FALSE */
    if (CAN_READ(inputBuffer, FIVE)
            && (strncmp((const char*) BUFFER_AT_OFFSET(inputBuffer), "FALSE",
                    FIVE) == ZERO)) {
        item->type = CJSON_FALSE;
        inputBuffer->offset += FIVE;
        return TRUE;
    }
    /* TRUE */
    if (CAN_READ(inputBuffer, FOUR)
            && (strncmp((const char*) BUFFER_AT_OFFSET(inputBuffer), "TRUE", FOUR)
                    == ZERO)) {
        item->type = CJSON_TRUE;
        item->valueint = ONE;
        inputBuffer->offset += FOUR;
        return TRUE;
    }
    /* string */
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == '\"')) {
        return parseString(item, inputBuffer);
    }
    /* number */
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && ((BUFFER_AT_OFFSET(inputBuffer)[ZERO] == '-')
                    || ((BUFFER_AT_OFFSET(inputBuffer)[ZERO] >= '0')
                            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] <= '9')))) {
        return parseNumber(item, inputBuffer);
    }
    /* array */
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == '[')) {
        return parseArray(item, inputBuffer);
    }
    /* object */
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == '{')) {
        return parseObject(item, inputBuffer);
    }

    return FALSE;
}

/* Render a value to text. */
static CJSON_bool printValue(const CJSON * const item,
        printbuffer * const outputBuffer) {
    unsigned char *output = NULL;

    if ((item == NULL) || (outputBuffer == NULL)) {
        return FALSE;
    }

    switch ((item->type) & CJSON_MAX) {
    case CJSON_NULL:
        output = ensure(outputBuffer, sizeof("null"));
        if (output == NULL) {
            return FALSE;
        }
        strncpy((char*) output, "null", sizeof("null"));
        return TRUE;

    case CJSON_FALSE:
        output = ensure(outputBuffer, sizeof("FALSE"));
        if (output == NULL) {
            return FALSE;
        }
        strncpy((char*) output, "FALSE", sizeof("FALSE"));
        return TRUE;

    case CJSON_TRUE:
        output = ensure(outputBuffer, sizeof("TRUE"));
        if (output == NULL) {
            return FALSE;
        }
        strncpy((char*) output, "TRUE", sizeof("TRUE"));
        return TRUE;

    case CJSON_NUMBER:
        return printNumber(item, outputBuffer);

    case CJSON_RAW: {
        size_t rawLength = ZERO;
        if (item->valuestring == NULL) {
            return FALSE;
        }

        rawLength = strlen(item->valuestring) + sizeof("");
        output = ensure(outputBuffer, rawLength);
        if (output == NULL) {
            return FALSE;
        }
        memcpy(output, item->valuestring, rawLength);
        return TRUE;
    }

    case CJSON_STRING:
        return printString(item, outputBuffer);

    case CJSON_ARRAY:
        return printArray(item, outputBuffer);

    case CJSON_OBJECT:
        return printObject(item, outputBuffer);

    default:
        return FALSE;
    }
}

/* Build an array from input text. */
static CJSON_bool parseArray(CJSON * const item,
        parse_buffer * const inputBuffer) {
    CJSON *head = NULL;
    CJSON *currentItem = NULL;

    if (inputBuffer->depth >= CJSON_NESTING_LIMIT) {
        return FALSE;
    }
    inputBuffer->depth++;

    if (BUFFER_AT_OFFSET(inputBuffer)[ZERO] != '[') {
        /* not an array */
        goto fail;
    }

    inputBuffer->offset++;
    bufferSkipWhitespace(inputBuffer);
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == ']')) {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)) {
        inputBuffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    inputBuffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        CJSON *newItem = cJsonNewItem(&(inputBuffer->hooks));
        if (newItem == NULL) {
            goto fail;
            /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            currentItem = head = newItem;
        } else {
            /* add to the end and advance */
            currentItem->next = newItem;
            newItem->prev = currentItem;
            currentItem = newItem;
        }

        /* parse next value */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseValue(currentItem, inputBuffer)) {
            goto fail;
            /* failed to parse value */
        }
        bufferSkipWhitespace(inputBuffer);
    } while (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == ','));

    if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)
            || BUFFER_AT_OFFSET(inputBuffer)[ZERO] != ']') {
        goto fail;
        /* expected end of array */
    }

    success: inputBuffer->depth--;

    item->type = CJSON_ARRAY;
    item->child = head;

    inputBuffer->offset++;

    return TRUE;

    fail: if (head != NULL) {
        cJsonDelete(head);
    }

    return FALSE;
}

/* Render an array to text */
static CJSON_bool printArray(const CJSON * const item,
        printbuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    size_t length = ZERO;
    CJSON *currentElement = item->child;

    if (outputBuffer == NULL) {
        return FALSE;
    }

    /* Compose the output array. */
    /* opening square bracket */
    outputPointer = ensure(outputBuffer, ONE);
    if (outputPointer == NULL) {
        return FALSE;
    }

    *outputPointer = '[';
    outputBuffer->offset++;
    outputBuffer->depth++;

    while (currentElement != NULL) {
        if (!printValue(currentElement, outputBuffer)) {
            return FALSE;
        }
        updateOffset(outputBuffer);
        if (currentElement->next) {
            length = (size_t)(outputBuffer->format ? TWO : ONE);
            outputPointer = ensure(outputBuffer, length + ONE);
            if (outputPointer == NULL) {
                return FALSE;
            }
            *outputPointer++ = ',';
            if (outputBuffer->format) {
                *outputPointer++ = ' ';
            }
            *outputPointer = '\0';
            outputBuffer->offset += length;
        }
        currentElement = currentElement->next;
    }

    outputPointer = ensure(outputBuffer, TWO);
    if (outputPointer == NULL) {
        return FALSE;
    }
    *outputPointer++ = ']';
    *outputPointer = '\0';
    outputBuffer->depth--;

    return TRUE;
}

/* Build an object from the text. */
static CJSON_bool parseObject(CJSON * const item,
        parse_buffer * const inputBuffer) {
    CJSON *head = NULL;
    CJSON *currentItem = NULL;

    if (inputBuffer->depth >= CJSON_NESTING_LIMIT) {
        return FALSE;
    }
    inputBuffer->depth++;

    if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)
            || (BUFFER_AT_OFFSET(inputBuffer)[ZERO] != '{')) {
        goto fail;
        /* not an object */
    }

    inputBuffer->offset++;
    bufferSkipWhitespace(inputBuffer);
    if (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == '}')) {
        goto success;
        /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)) {
        inputBuffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    inputBuffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        CJSON *newItem = cJsonNewItem(&(inputBuffer->hooks));
        if (newItem == NULL) {
            goto fail;
            /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            currentItem = head = newItem;
        } else {
            /* add to the end and advance */
            currentItem->next = newItem;
            newItem->prev = currentItem;
            currentItem = newItem;
        }

        /* parse the name of the child */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseString(currentItem, inputBuffer)) {
            goto fail;
            /* failed to parse name */
        }
        bufferSkipWhitespace(inputBuffer);

        /* swap valuestring and string, because we parsed the name */
        currentItem->string = currentItem->valuestring;
        currentItem->valuestring = NULL;

        if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)
                || (BUFFER_AT_OFFSET(inputBuffer)[ZERO] != ':')) {
            goto fail;
            /* invalid object */
        }

        /* parse the value */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseValue(currentItem, inputBuffer)) {
            goto fail;
            /* failed to parse value */
        }
        bufferSkipWhitespace(inputBuffer);
    } while (CAN_ACCESS_AT_INDEX(inputBuffer, ZERO)
            && (BUFFER_AT_OFFSET(inputBuffer)[ZERO] == ','));

    if (CANNOT_ACCESS_AT_INDEX(inputBuffer, ZERO)
            || (BUFFER_AT_OFFSET(inputBuffer)[ZERO] != '}')) {
        goto fail;
        /* expected end of object */
    }

    success: inputBuffer->depth--;

    item->type = CJSON_OBJECT;
    item->child = head;

    inputBuffer->offset++;
    return TRUE;

    fail: if (head != NULL) {
        cJsonDelete(head);
    }

    return FALSE;
}

/* Render an object to text. */
static CJSON_bool printObject(const CJSON * const item,
        printbuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    size_t length = ZERO;
    CJSON *currentItem = item->child;

    if (outputBuffer == NULL) {
        return FALSE;
    }

    /* Compose the output: */
    length = (size_t)(outputBuffer->format ? TWO : ONE);
    outputPointer = ensure(outputBuffer, length + ONE);
    if (outputPointer == NULL) {
        return FALSE;
    }

    *outputPointer++ = '{';
    outputBuffer->depth++;
    if (outputBuffer->format) {
        *outputPointer++ = '\n';
    }
    outputBuffer->offset += length;

    while (currentItem) {
        if (outputBuffer->format) {
            size_t i;
            outputPointer = ensure(outputBuffer, outputBuffer->depth);
            if (outputPointer == NULL) {
                return FALSE;
            }
            for (i = ZERO; i < outputBuffer->depth; i++) {
                *outputPointer++ = '\t';
            }
            outputBuffer->offset += outputBuffer->depth;
        }

        /* print key */
        if (!printStringPtr((unsigned char*) currentItem->string,
                outputBuffer)) {
            return FALSE;
        }
        updateOffset(outputBuffer);

        length = (size_t)(outputBuffer->format ? TWO : ONE);
        outputPointer = ensure(outputBuffer, length);
        if (outputPointer == NULL) {
            return FALSE;
        }
        *outputPointer++ = ':';
        if (outputBuffer->format) {
            *outputPointer++ = '\t';
        }
        outputBuffer->offset += length;

        /* print value */
        if (!printValue(currentItem, outputBuffer)) {
            return FALSE;
        }
        updateOffset(outputBuffer);

        /* print comma if not last */
        length = ((size_t)(outputBuffer->format ? ONE : ZERO)
                + (size_t)(currentItem->next ? ONE : ZERO));
        outputPointer = ensure(outputBuffer, length + ONE);
        if (outputPointer == NULL) {
            return FALSE;
        }
        if (currentItem->next) {
            *outputPointer++ = ',';
        }

        if (outputBuffer->format) {
            *outputPointer++ = '\n';
        }
        *outputPointer = '\0';
        outputBuffer->offset += length;

        currentItem = currentItem->next;
    }

    outputPointer = ensure(outputBuffer,
            outputBuffer->format ? (outputBuffer->depth + ONE) : TWO);
    if (outputPointer == NULL) {
        return FALSE;
    }
    if (outputBuffer->format) {
        size_t i;
        for (i = ZERO; i < (outputBuffer->depth - ONE); i++) {
            *outputPointer++ = '\t';
        }
    }
    *outputPointer++ = '}';
    *outputPointer = '\0';
    outputBuffer->depth--;

    return TRUE;
}

/* Utility for array list handling. */
static void suffixObject(CJSON *prev, CJSON *item) {
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static CJSON_bool addItemToArray(CJSON *array, CJSON *item) {
    CJSON *child = NULL;

    if ((item == NULL) || (array == NULL)) {
        return FALSE;
    }

    child = array->child;

    if (child == NULL) {
        /* list is empty, start new one */
        array->child = item;
    } else {
        /* append to the end */
        while (child->next) {
            child = child->next;
        }
        suffixObject(child, item);
    }

    return TRUE;
}

/* Add item to array/object. */
CJSON_PUBLIC(void) cJsonAddItemToArray(CJSON *array, CJSON *item) {
    addItemToArray(array, item);
}

/* helper function to cast away const */
static void* castAwayConst(const void* string) {
    return (void*) string;
}

static CJSON_bool addItemToObject(CJSON * const object,
        const char * const string, CJSON * const item,
        const internal_hooks * const hooks, const CJSON_bool constantKey) {
    char *newKey = NULL;
    unsigned int newType = CJSON_INVALID;

    if ((object == NULL) || (string == NULL) || (item == NULL)) {
        return FALSE;
    }

    if (constantKey) {
        newKey = (char*) castAwayConst(string);
        newType = item->type | CJSON_STRINGISCONST;
    } else {
        newKey = (char*) cJsonStrdup((const unsigned char*) string, hooks);
        if (newKey == NULL) {
            return FALSE;
        }

        newType = item->type & ~(unsigned int)CJSON_STRINGISCONST;
    }

    if (!(item->type & CJSON_STRINGISCONST) && (item->string != NULL)) {
        hooks->deallocate(item->string);
    }

    item->string = newKey;
    item->type = newType;

    return addItemToArray(object, item);
}

CJSON_PUBLIC(void) cJsonAddItemToObject(CJSON *object, const char *string, CJSON *item) {
    addItemToObject(object, string, item, &global_hooks, FALSE);
}

/* Create basic types: */
CJSON_PUBLIC(CJSON *) cJsonCreateNumber(double num)
{
    CJSON *item = cJsonNewItem(&global_hooks);
    if(item)
    {
        item->type = CJSON_NUMBER;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= (double)INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }

    return item;
}

CJSON_PUBLIC(CJSON *) cJsonCreateString(const char *string)
{
    CJSON *item = cJsonNewItem(&global_hooks);
    if(item)
    {
        item->type = CJSON_STRING;
        item->valuestring = (char*)cJsonStrdup((const unsigned char*)string, &global_hooks);
        if(!item->valuestring)
        {
            cJsonDelete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(CJSON *) cJsonCreateArray(void)
{
    CJSON *item = cJsonNewItem(&global_hooks);
    if(item)
    {
        item->type=CJSON_ARRAY;
    }

    return item;
}

CJSON_PUBLIC(CJSON *) cJsonCreateObject(void)
{
    CJSON *item = cJsonNewItem(&global_hooks);
    if (item)
    {
        item->type = CJSON_OBJECT;
    }

    return item;
}

///////////////////////////////////////////////////////////////////////

#define CSV_ERR_LONGLINE ZERO
#define CSV_ERR_NO_MEMORY ONE

static void freeCsvLine(char **parsed) {
    char **ptr;

    for (ptr = parsed; *ptr; ptr++) {
        free(*ptr);
    }

    free(parsed);
}

static int countFields(const char *line) {
    const char *ptr;
    int cnt;
    int fQuote;

    for (cnt = ONE, fQuote = ZERO, ptr = line; *ptr; ptr++) {
        if (fQuote) {
            if (*ptr == '\"') {
                if (ptr[ONE] == '\"') {
                    ptr++;
                    continue;
                }
                fQuote = ZERO;
            }
            continue;
        }

        switch (*ptr) {
        case '\"':
            fQuote = ONE;
            continue;
        case ',':
            cnt++;
            continue;
        default:
            continue;
        }
    }

    if (fQuote) {
        return -ONE;
    }

    return cnt;
}

static char **parseCsv(const char *line) {
    char **buf;
    char **bptr;
    char *tmp;
    char *tptr;
    const char *ptr;
    int fieldcnt;
    int fQuote;
    int fEnd;

    fieldcnt = countFields(line);

    if (fieldcnt == -ONE) {
        return NULL;
    }

    buf = (char **) malloc(sizeof(char*) * (fieldcnt + ONE));

    if (!buf) {
        return NULL;
    }

    tmp = (char *) malloc(strlen(line) + ONE);
    if (!tmp) {
        free(buf);
        return NULL;
    }

    bptr = buf;

    for (ptr = line, fQuote = ZERO, *tmp = '\0', tptr = tmp, fEnd = ZERO;; ptr++) {
        if (fQuote) {
            if (!*ptr) {
                break;
            }

            if (*ptr == '\"') {
                if (ptr[ONE] == '\"') {
                    *tptr++ = '\"';
                    ptr++;
                    continue;
                }
                fQuote = ZERO;
            } else {
                *tptr++ = *ptr;
            }

            continue;
        }

        switch (*ptr) {
        case '\"':
            fQuote = ONE;
            continue;
        case '\0':
            fEnd = ONE;
        case ',':
            *tptr = '\0';
            *bptr = strdup(tmp);

            if (!*bptr) {
                for (bptr--; bptr >= buf; bptr--) {
                    free(*bptr);
                }
                free(buf);
                free(tmp);

                return NULL;
            }

            bptr++;
            tptr = tmp;

            if (fEnd) {
                break;
            } else {
                continue;
            }

        default:
            *tptr++ = *ptr;
            continue;
        }

        if (fEnd) {
            break;
        }
    }

    *bptr = NULL;
    free(tmp);
    return buf;
}

#define READ_BLOCK_SIZE 65536
#define QUICK_GETC( ch, fp )\
do\
{\
    if ( readPtr == readEnd )\
    {\
        freadLen = fread( readBuf, sizeof(char), READ_BLOCK_SIZE, fp );\
        if ( freadLen < READ_BLOCK_SIZE )\
            readBuf[freadLen] = '\0';\
        readPtr = readBuf;\
    }\
    ch = *readPtr++;\
}\
while(ZERO)

static char *freadCsvLine(FILE *fp, int maxLineSize, int *done, int *err) {
    static FILE *bookmark;
    static char readBuf[READ_BLOCK_SIZE];
    static char *readPtr;
    static char *readEnd;
    static int freadLen;
    static int prevMaxLineSize = -ONE;
    static char *buf;
    char *bptr;
    char *limit;
    char ch;
    int fQuote;

    if (maxLineSize > prevMaxLineSize) {
        if (prevMaxLineSize != -ONE) {
            free(buf);
        }
        buf = (char *) malloc(maxLineSize + ONE);
        if (!buf) {
            *err = CSV_ERR_NO_MEMORY;
            prevMaxLineSize = -ONE;
            return NULL;
        }
        prevMaxLineSize = maxLineSize;
    }
    bptr = buf;
    limit = buf + maxLineSize;

    if (bookmark != fp) {
        readPtr = readEnd = readBuf + READ_BLOCK_SIZE;
        bookmark = fp;
    }

    for (fQuote = ZERO;;) {
        QUICK_GETC(ch, fp);

        if (!ch || (ch == '\n' && !fQuote)) {
            break;
        }

        if (bptr >= limit) {
            free(buf);
            *err = CSV_ERR_LONGLINE;
            return NULL;
        }
        *bptr++ = ch;

        if (fQuote && ch == '\"') {
            QUICK_GETC(ch, fp);

            if (ch != '\"') {
                if (!ch || ch == '\n') {
                    break;
                }
                fQuote = ZERO;
            }
            *bptr++ = ch;
        }

        if (ch == '\"') {
            fQuote = ONE;
        }
    }

    *done = !ch;
    *bptr = '\0';
    return strdup(buf);
}

///////////////////////////////////////////////////////////////////////

//#define DEBUG_CSV
//#define DEBUG_PATH
//#define DEBUG_TIME
//#define DEBUG_GROUP
//#define DEBUG_SOLVE
//#define DEBUG_ATTATCH
#define DEBUG_JSON

#define MAGIC   (3)

typedef enum {
    csv_type_problem_id,
    csv_type_title,
    csv_type_dscription,
    csv_type_reported_date,
    csv_type_group_in_charge,
    csv_type_feature,
    csv_type_state,
    csv_type_severity,
    csv_type_top_importance,
    csv_type_release,
    csv_type_rd_information,
    csv_type_author,
    csv_type_author_group,
    csv_type_attached_prs,
    csv_type_responsible_person,
    csv_type_revision_history,
    csv_type_target_build,
} csv_type_t;

typedef struct {
    char **parsed;
} csv_entry_t;

typedef struct {
    int flag;//1:used
    char *from;
    char *to;
} path_entry_t;

const char *ece_group[] = {
    "ECE_DEV_FOU",
    "ECE_DEV_FOU_OAM_MZ",
    "ECE_DEV_FOU_OAM_MZ_EXT",
    "MANO_HZH_MZOM",
    "MANO_HZH_MZOM_EXT",
    "ECE_DEV_FOU_UP_L2_HI",
    "ECE_DEV_FOU_UP_L2_LO",
};

static int readInfoFromCsv(char *file, csv_entry_t **array, int *size) {
    int err = ZERO;
    int done = ZERO;
    char **parsed = NULL;
    char *strbuf = NULL;
    csv_entry_t *tmpArray = NULL;
    int tmpSize = ZERO;
    int firstLine = ZERO;

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "fopen err\n");
        return -ONE;
    }

    while (ONE) {
        strbuf = freadCsvLine(fp, FIVE * TEN * 1024 * 1024, &done, &err);
        if (strbuf == NULL || err != ZERO) {
            fprintf(stderr, "freadCsvLine err\n");
            err = -ONE;
            goto end;
        }

        if (done == ONE) {
            break;
        }

        parsed = parseCsv(strbuf);
        if (parsed == NULL) {
            fprintf(stderr, "parseCsv err\n");
            err = -ONE;
            goto end;
        }

        if (firstLine == ZERO) {

            if (strcmp(parsed[0], "Problem ID") == ZERO) {
                firstLine = ONE;
            }

            free(strbuf);
            continue;
        }


#ifdef DEBUG_CSV
        int i = ZERO;
        while (parsed[i]) {
            printf("[%d]:%s\n", i, parsed[i]);
            i++;
        }
#endif

        tmpArray = (csv_entry_t *) realloc(tmpArray,
                sizeof(csv_entry_t) * (tmpSize + ONE));
        if (tmpArray == NULL) {
            fprintf(stderr, "realloc err\n");
            err = -ONE;
            goto end;
        }

        tmpArray[tmpSize].parsed = parsed;
        tmpSize++;

        free(strbuf);
    }

    *array = tmpArray;
    *size = tmpSize;

end:
    free(strbuf);
    fclose(fp);
    return err;
}

static int compareWithID(const void *s1, const void *s2) {

    const csv_entry_t *e1 = (const csv_entry_t *)s1;
    const csv_entry_t *e2 = (const csv_entry_t *)s2;

    return strcmp(e1->parsed[csv_type_problem_id], e2->parsed[csv_type_problem_id]);
}

static int sortCsvByID(csv_entry_t *array, int size) {

    qsort(array, size, sizeof(csv_entry_t), compareWithID);

    return ZERO;
}

static void cleanupPath(char *path) {

    int len = strlen(path);

    while(path[len - 1] == ' ') {
        path[len - 1] = 0;
        len--;
    }
}

static int sortPath(path_entry_t *tmpArray, int tmpSize, int cur, char *from, char **tmpPaths, int *fuckSize)
{
    int i;
    int realSize = *fuckSize;

    if (cur == tmpSize) {

        for (i = 0; i < tmpSize; i++) {

            if (tmpArray[i].flag != MAGIC) {
                return -ONE;
            }
        }

        return ZERO;
    }
    else if (cur == 0) {

        tmpPaths[tmpSize - 1 - realSize] = tmpArray[cur].from;
        realSize++;
#ifdef DEBUG_PATH
        printf("Add %s\n", tmpArray[cur].from);
#endif
        from = tmpArray[cur].from;
        tmpArray[cur].flag = MAGIC;
        *fuckSize = realSize;
        return sortPath(tmpArray, tmpSize, cur + 1, from, tmpPaths, fuckSize);
    }
    else {

        for (i = 0; i < tmpSize; i++) {
#ifdef DEBUG_PATH
            printf("check[%d] %s [%d]%s flag:%d\n", cur, from, i, tmpArray[i].to, tmpArray[i].flag);
#endif
            if (tmpArray[i].flag == MAGIC) {
                continue;
            }

            if (from && strcmp(from, tmpArray[i].to) == ZERO) {
                tmpPaths[tmpSize - 1 - realSize] = tmpArray[i].from;
                realSize++;
#ifdef DEBUG_PATH
                printf("Add %s\n", tmpArray[i].from);
#endif
                from = tmpArray[i].from;
                tmpArray[i].flag = MAGIC;
                *fuckSize = realSize;
                if (sortPath(tmpArray, tmpSize, cur + 1, from, tmpPaths, fuckSize) == ZERO) {
                    return ZERO;
                } else {
                    tmpArray[i].flag = 0;
                    realSize--;
                }
            }
        }

        return -ONE;
    }

    return ZERO;
}

static int getTransferPath(char *myself, char *string, char ***array, int *size) {
    int i;
    char *p = string;
    const char *need1 = "The group in charge changed from ";
    const char *need2 = " to ";
    const char *need3 = " To ";
    int tmpSize = ZERO;
    int realSize = ZERO;
    path_entry_t *tmpArray = NULL;
    char **tmpPaths = NULL;
    char *from = NULL;
    char *to = NULL;

    tmpArray = (path_entry_t *) malloc(sizeof(path_entry_t) * (tmpSize + ONE));
    if (tmpArray == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    cleanupPath(myself);
    tmpArray[tmpSize].from = strdup(myself);
    tmpArray[tmpSize].to = strdup("fuck");
    tmpSize++;

    while (ONE) {

        p = strstr(p, need1);
        if (p == NULL) {
            break;
        }
        p += strlen(need1);
        from = p;

        char *tmp1 = strstr(p, need2);
        char *tmp2 = strstr(p, need3);
        if (tmp1 == NULL && tmp2 == NULL) {
            break;
        } else if (tmp1 == NULL) {
            p = tmp2;
        } else if (tmp2 == NULL) {
            p = tmp1;
        } else {
            p = tmp1 < tmp2 ? tmp1 : tmp2;
        }
        *p = ZERO;

        p += strlen(need2);
        to = p;

        while (ONE) {
            if (*p == '.' || *p == ',') {
                *p++ = ZERO;
                break;
            }

            if (*p == ' ' && *(p + 1) == '2' && *(p + 2) == '0') {
                *p++ = ZERO;
            }

            p++;
        }

        cleanupPath(from);
        cleanupPath(to);

        //from
#ifdef DEBUG_PATH
        printf("from '%s' to '%s'\n", from, to);
#endif

        //1.skip the same organization
        if (strcmp(from, to) == ZERO) {
            continue;
        }

        //update
        tmpArray = (path_entry_t*) realloc(tmpArray, sizeof(path_entry_t) * (tmpSize + ONE));
        if (tmpArray == NULL) {
            fprintf(stderr, "realloc err\n");
            return -ONE;
        }

        tmpArray[tmpSize].flag = 0;
        tmpArray[tmpSize].from = strdup(from);
        tmpArray[tmpSize].to = strdup(to);
        tmpSize++;
    }

    //sort path
    tmpPaths = (char **) malloc(sizeof(char *) * (tmpSize + ONE));
    if (tmpPaths == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

     sortPath(tmpArray, tmpSize, 0, NULL, tmpPaths, &realSize);
     memcpy(tmpPaths, &tmpPaths[tmpSize - realSize], realSize * sizeof(char *));
///////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_PATH
    for (i = 0; i < realSize; i++) {
        printf("path:'%s'\n",  tmpPaths[i]);
    }
#endif

    *array = tmpPaths;
    *size = realSize;

    for (i = 0; i < tmpSize; i++) {
        free(tmpArray[i].to);
    }
    free(tmpArray);

    return ZERO;
}

static int findInGroup(char *transferPath) {
    int i;

    for (i = ONE; i < (int)(sizeof(ece_group)/sizeof(char *)); i++) {

        if (strcmp(ece_group[i], transferPath) == ZERO) {
            return ZERO;
        }
    }

    return -ONE;
}

//A PR can be transferred by organization internal arrangements
static int getTransferGroupPath(char **transferPath, int transferSize,
        char ***transferGroupPath, int *transferGroupSize) {
    int i, j;
    int tmpSize = ZERO;
    char **tmpArray = NULL;
    int parent = -ONE;

    tmpArray = (char **) realloc(tmpArray, sizeof(char *) * transferSize);
    if (tmpArray == NULL) {
        fprintf(stderr, "realloc err\n");
        return -ONE;
    }

    for (i = ZERO; i < transferSize; i++) {

        for (parent = -ONE, j = i; j < transferSize; j++) {

            if (strcmp(transferPath[j], ece_group[ZERO]) == ZERO) {
                parent = j;
                continue;
            }

            if (findInGroup(transferPath[j]) != ZERO) {
                break;
            }
        }

        if (i != j) {

            //rule2 head
            if (i == 0) {
                tmpArray[tmpSize] = transferPath[j - 1];
            } else {
                //rule1 middle
                tmpArray[tmpSize] = transferPath[i];
            }

            tmpSize++;
        }

        //update the one which is not in group
        if (j < transferSize) {
            tmpArray[tmpSize] = transferPath[j];
            tmpSize++;
        }

        i = j;
    }

    //rule3 tail
    if (tmpSize == ONE) {
        tmpArray[tmpSize - 1] = transferPath[transferSize - ONE];
    }

#ifdef DEBUG_GROUP
    for (i = 0; i < tmpSize; i++) {
        printf("group:%s\n", tmpArray[i]);
    }
#endif

    *transferGroupPath = tmpArray;
    *transferGroupSize = tmpSize;

    return ZERO;
}

static int findPathInGroup(char *path, char **group, int first, int last) {

    int i;

    for (i = first; i < last; i++) {

        if (strcmp(path, group[i]) == ZERO) {
            return i;
        }
    }

    return -ONE;
}

static int getSolvingPath(char **transferPath, int transferSize,
        char ***solvingPath, int *solvingSize) {
    int i, j, k;
    int tmpSize = ZERO;
    char **tmpArray = NULL;

    tmpArray = (char **) malloc(sizeof(char *) * transferSize);
    if (tmpArray == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    for (i = ZERO; i < transferSize; i++) {

        //check i in tmpArray?  zhijie
        for (j = ZERO; j < tmpSize; j++) {

            if (strcmp(transferPath[i], tmpArray[j]) == ZERO) {
#ifdef DEBUG_SOLVE
                for (k = j; k < tmpSize; k++) {
                    printf("remove1:%s\n", tmpArray[k]);
                }
#endif
                tmpSize = j;
                goto end;
            }
        }

        //check i in dropArray? jianjie
        for (j = i - 1; j >= 0; j--) {

            //find one in origin
            if (strcmp(transferPath[i], transferPath[j]) == ZERO) {

                for (k = i - 1; k >= 0; k--) {

                    if (findPathInGroup(transferPath[k], transferPath, 0, k - 1) >= ZERO) {
#ifdef DEBUG_SOLVE
                        int l;
                        for (l = tmpSize - (i - k - 1); l < tmpSize; l++) {
                            printf("remove2:%s\n", tmpArray[l]);
                        }
#endif
                        if (tmpSize - (i - k - 1) < tmpSize) {
                            tmpSize -= (i - k - 1);
                        }
                        goto end;
                    }
                }

            }
        }

end:
        tmpArray[tmpSize] = transferPath[i];
        tmpSize++;
#ifdef DEBUG_SOLVE
        printf("add1 :%s\n", transferPath[i]);
#endif
    }

    *solvingPath = tmpArray;
    *solvingSize = tmpSize;

    return ZERO;
}

static int getPingpongTime(char **transferPath, int transferSize,
        int *pingpongTime) {
    int i;
    int j;
    int tmpTime = ZERO;
    int tmpSize = ZERO;
    char **tmpArray = NULL;

    tmpArray = (char **) malloc(sizeof(char *) * transferSize);
    if (tmpArray == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    for (i = ZERO; i < transferSize; i++) {

        for (j = ZERO; j < tmpSize; j++) {
            if (strcmp(tmpArray[j], transferPath[i]) == ZERO) {
                break;
            }
        }

        if (j == tmpSize) {
            tmpArray[tmpSize] = transferPath[i];
            tmpSize++;
        }
    }

    for (i = ZERO; i < transferSize; i++) {

        for (j = ZERO; j < tmpSize; j++) {
            if (strcmp(tmpArray[j], transferPath[i]) == ZERO) {
                tmpTime++;
            }
        }
    }

    *pingpongTime = tmpTime - tmpSize;
    free(tmpArray);

    return ZERO;
}

static int getAttachedTo(char *myself, char *string, csv_entry_t *array,
        int size, char **result) {

    int i;
    char *tmpString = strdup(string);
    char *ptr = tmpString;
    char *miniitem = myself;

#ifdef DEBUG_ATTATCH
    printf("PR ID:%s\n", myself);
#endif

    while (ONE) {

        char *item = ptr;

        while(1) {

            if (*ptr == ',') {
                *ptr = 0;
                ptr += 2;
                break;
            }

            if (*ptr == '\0') {
                break;
            }

            ptr++;
        }

        if (item == ptr) {
            break;
        }

        //find item in array
        for (i = ZERO; i < size; i++) {

            if (strcmp(array[i].parsed[csv_type_problem_id], item) == ZERO) {
                break;
            }
        }

        if (i == size) {
            continue;
        }

        miniitem = strcmp(miniitem, item) > 0 ? item : miniitem;

#ifdef DEBUG_ATTATCH
        printf("Attached PRs:%s\n", item);
#endif
    }

    if (strcmp(miniitem, myself) == ZERO) {
        *result = NULL;
    } else {
        for (i = ZERO; i < size; i++) {

            if (strcmp(array[i].parsed[csv_type_problem_id], miniitem) == ZERO) {
                miniitem = array[i].parsed[csv_type_problem_id];
                break;
            }
        }

        if (i == size) {
            miniitem = NULL;
        } else {
            char *tmpp = NULL;
            getAttachedTo(
                    array[i].parsed[csv_type_problem_id],
                    array[i].parsed[csv_type_attached_prs],
                    array, size, &tmpp);
            miniitem = tmpp ? tmpp : miniitem;
        }

        *result = miniitem;
    }

    free(tmpString);

    return ZERO;
}

static int convertCsvToJson(csv_entry_t *array, int size, CJSON **result) {

    int index = ZERO;
    int count = ZERO;
    CJSON *json;
    CJSON *records;
    CJSON *total;

    json = cJsonCreateObject();
    if (json == NULL) {
        return -ONE;
    }

    records = cJsonCreateArray();
    if (records == NULL) {
        return -ONE;
    }
    cJsonAddItemToObject(json, "records", records);

    for (index = ZERO; index < size; ++index) {
        CJSON *entry = cJsonCreateObject();
        if (entry == NULL) {
            return -ONE;
        }
        cJsonAddItemToArray(records, entry);

        ///////////////////////////////////////////////////////////////////////

        CJSON *problemId = cJsonCreateString(
                array[index].parsed[csv_type_problem_id]);
        if (problemId == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "PR ID", problemId);

        ///////////////////////////////////////////////////////////////////////

        CJSON *groupInCharge = cJsonCreateString(
                array[index].parsed[csv_type_group_in_charge]);
        if (groupInCharge == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Group In Charge", groupInCharge);

        ///////////////////////////////////////////////////////////////////////

        CJSON *reportedDate = cJsonCreateString(
                array[index].parsed[csv_type_reported_date]);
        if (reportedDate == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Reported Date", reportedDate);

        ///////////////////////////////////////////////////////////////////////

        CJSON *author = cJsonCreateString(
                array[index].parsed[csv_type_author]);
        if (author == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Author", author);

        ///////////////////////////////////////////////////////////////////////

        CJSON *authorGroup = cJsonCreateString(
                array[index].parsed[csv_type_author_group]);
        if (authorGroup == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Author Group", authorGroup);

        ///////////////////////////////////////////////////////////////////////

        CJSON *state = cJsonCreateString(array[index].parsed[csv_type_state]);
        if (state == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "State", state);

        ///////////////////////////////////////////////////////////////////////

        int transferTmpSize = ZERO;
        char **transferTmpPaths = NULL;
        getTransferPath(array[index].parsed[csv_type_group_in_charge],
                array[index].parsed[csv_type_revision_history],
                &transferTmpPaths, &transferTmpSize);

        int transferSize = ZERO;
        char **transferPaths = NULL;
        getTransferGroupPath(transferTmpPaths, transferTmpSize,
                &transferPaths, &transferSize);

        CJSON *transferPath = cJsonCreateArray();
        if (transferPath == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Transfer Path", transferPath);

        for (count = ZERO; count < transferSize; ++count) {
            CJSON *tmp = cJsonCreateString(transferPaths[count]);
            if (tmp == NULL) {
                return -ONE;
            }
            cJsonAddItemToObject(transferPath, "", tmp);
        }

        ///////////////////////////////////////////////////////////////////////

        CJSON *transferTime = cJsonCreateNumber(transferSize);
        if (transferTime == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Transfer times", transferTime);

        ///////////////////////////////////////////////////////////////////////

        int solvingSize = ZERO;
        char **solvingPaths = NULL;
        getSolvingPath(transferPaths, transferSize, &solvingPaths,
                &solvingSize);

        CJSON *solvingPath = cJsonCreateArray();
        if (solvingPath == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Solving Path", solvingPath);

        for (count = ZERO; count < solvingSize; ++count) {
            CJSON *tmp = cJsonCreateString(solvingPaths[count]);
            if (tmp == NULL) {
                return -ONE;
            }
            cJsonAddItemToObject(solvingPath, "", tmp);
        }

        ///////////////////////////////////////////////////////////////////////

        int pingpongTime = ZERO;
        getPingpongTime(transferPaths, transferSize, &pingpongTime);

        CJSON *pingpongTimes = cJsonCreateNumber(pingpongTime);
        if (pingpongTimes == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Pingpong times", pingpongTimes);

        ///////////////////////////////////////////////////////////////////////

        CJSON *attachedPrs = cJsonCreateString(
                array[index].parsed[csv_type_attached_prs]);
        if (attachedPrs == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Attached PRs", attachedPrs);

        ///////////////////////////////////////////////////////////////////////

        char *attachedToStr = NULL;
        getAttachedTo(array[index].parsed[csv_type_problem_id],
                array[index].parsed[csv_type_attached_prs], array, size,
                &attachedToStr);

        CJSON *attached = cJsonCreateString(attachedToStr ? "yes" : "no");
        if (attached == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Attached", attached);

        ///////////////////////////////////////////////////////////////////////

        CJSON *attachedTo = cJsonCreateString(
                attachedToStr ? (const char *) attachedToStr : "");
        if (attachedTo == NULL) {
            return -ONE;
        }
        cJsonAddItemToObject(entry, "Attached To", attachedTo);

        ///////////////////////////////////////////////////////////////////////

        for (count = ZERO; count < transferTmpSize; count++) {
            free(transferTmpPaths[count]);
        }
        free(transferTmpPaths);
        if (transferPaths != transferTmpPaths) {
            free(transferPaths);
        }
        free(solvingPaths);
    }

    total = cJsonCreateNumber(size);
    if (total == NULL) {
        return -ONE;
    }
    cJsonAddItemToObject(json, "total", total);

    *result = json;
    return ZERO;
}

static int writeInfoToJson(CJSON *result) {

    char *string = cJsonPrint(result);
    if (string == NULL) {
        fprintf(stderr, "Failed to print result.\n");
        return -ONE;
    }

    fprintf(stdout, "%s", string);
    cJsonDelete(result);

    return ZERO;
}

int main(int argc, char **argv) {

    int i;
    int ret = ZERO;
    csv_entry_t *csvArray = NULL;
    int csvSize = ZERO;
    CJSON *json = NULL;

    ret = readInfoFromCsv(argv[ONE], &csvArray, &csvSize);
    if (ret != ZERO) {
        goto end;
    }

    ret = sortCsvByID(csvArray, csvSize);
    if (ret != ZERO) {
        goto end;
    }

    ret = convertCsvToJson(csvArray, csvSize, &json);
    if (ret != ZERO) {
        goto end;
    }

    ret = writeInfoToJson(json);
    if (ret != ZERO) {
        goto end;
    }


end:
    for (i = ZERO; i < csvSize; i++) {
        if (csvArray[i].parsed) {
            freeCsvLine(csvArray[i].parsed);
        }
    }

    if (csvArray) {
        free(csvArray);
    }

    return ret;
}

