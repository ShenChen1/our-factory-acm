#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <cstring>
#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>

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
} number_e;

typedef struct
{
    int consonant_len;
	const char *consonant_array;//b c d g k p q t
    
    int male_endstring_len;
    const char **male_endstring_array;//-an -ian -in -on -r -ah -as -o -os -us

    int female_endstring_len;
    const char **female_endstring_array;//-a -ia -ie -ine -e -es -is -x -z   

} rule_t;

static const char s_vowel[] = 
{
    'a','e','i','o','u','y'
};

static const char s_consonant[] = 
{
    'b','c','d','g','k','p','q','t','s','l'
};

static const char *s_male_end[] = 
{
    "an",
    "ian",
    "in",
    "on",
    "r",
    "ah",
    "as",
    "o",
    "os",
    "us",
};

static const char *s_female_end[] = 
{
    "a",
    "ia",
    "ie",
    "ine",
    "e",
    "es",
    "is",
    "x",
    "z",
};

static rule_t s_rule = {0};

static int doMaleRule1(char *str, int len)
{
    int i;
    char *ptr;

    //check if male's name end string
    for (i = 0; i < s_rule.male_endstring_len; i++)
    {
        ptr = strstr(str, s_rule.male_endstring_array[i]);
        if (ptr + strlen(s_rule.male_endstring_array[i]) == str + len)
        {
            break;
        }
    }

    if (i == s_rule.male_endstring_len)
    {
        return -1;
    }

    //skip vowel
    while (ptr == str)
    {
        ptr = ptr-1;

        for (i = 0; i < static_cast<int>(sizeof(s_vowel)); i++)
        {
            if (*ptr == s_vowel[i])
            {
                break;
            }
        }

        if (i == sizeof(s_vowel))
        {
            break;
        }
    }

    //just one char left
    if (ptr == str + 1)
    {
        return 0;
    }

    //check if male's name consonant
    for (i = 0; i < s_rule.consonant_len; i++)
    {
        if (*(ptr-1) == s_rule.consonant_array[i])
        {
            return 0;
        }
    }

    return -1;
}


static int doMaleRule2(char *str, int len)
{
    int i;
    char *ptr = str + len;

    //skip vowel
    while (ptr == str)
    {
        ptr = ptr-1;

        for (i = 0; i < static_cast<int>(sizeof(s_vowel)); i++)
        {
            if (*ptr == s_vowel[i])
            {
                break;
            }
        }

        if (i == sizeof(s_vowel))
        {
            break;
        }
    }

    //just one char left
    if (ptr == str + 1)
    {
        return 0;
    }

    //check if male's name consonant
    for (i = 0; i < s_rule.consonant_len; i++)
    {
        if (*(ptr-1) == s_rule.consonant_array[i])
        {
            return 0;
        }
    }

    return -1;
}

static int doMaleRule3(char *str, int len)
{
    int i;
    char *ptr = str + len;

    //check if male's name consonant
    for (i = 0; i < s_rule.consonant_len; i++)
    {
        if (*(ptr-1) == s_rule.consonant_array[i])
        {
            return 0;
        }
    }

    return -1;
}

static int doMaleProcess(char *str, int len)
{
    int ret;

    ret = doMaleRule1(str, len);
    if (ret == 0)
    {
        return 0;
    }

    ret = doMaleRule2(str, len);
    if (ret == 0)
    {
        return 0;
    }

    ret = doMaleRule3(str, len);
    if (ret == 0)
    {
        return 0;
    }

    return -1;
}

static void doProcess(char *str, int len)
{
    int ret;

    while(!isalpha(str[len]))
    {        
        str[len] = '\0';
        len--;
    }

    ret = doMaleProcess(str, len);
    if (ret)
    {
        fprintf(stdout, "%s,%s\n", str, "male");
    }
    else
    {
        fprintf(stdout, "%s,%s\n", str, "female");
    }
}

static int praseFile(char *path)
{
    FILE *file = NULL;
    char buff[512];

    file = fopen(path , "r");
    if (file == NULL)
    {
        return -1;
    }

    while (NULL != fgets(buff, sizeof(buff), file))
    {
        doProcess(buff, strnlen(buff, sizeof(buff)));
    }

    fclose(file);

    return 0;
}


static void doInit()
{
    s_rule.consonant_len = sizeof(s_consonant);
    s_rule.consonant_array = s_consonant;

    s_rule.male_endstring_len = sizeof(s_male_end)/sizeof(char *);
    s_rule.male_endstring_array = s_male_end;

    s_rule.female_endstring_len = sizeof(s_female_end)/sizeof(char *);
    s_rule.female_endstring_array = s_female_end;
}


int main(int argc, char **argv)
{
    if ((argc != TWO) || (argv[1] == NULL))
    {
        return -1;
    }

    doInit();

    praseFile(argv[1]);

    return 0;
}
