#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <cstring>
#include <string>
#include <stdlib.h>

#define MAX        (32)

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
    int num;
    int letters[MAX];
} number_t;

typedef struct
{
    char opt;
    number_t x;
    number_t y;
    number_t z;

} equation_t;

typedef struct
{
    int num;
    char letters[MAX];
    int number[MAX];
    int notzero[MAX];

} letters_t;

static letters_t s_letters = {0};
static equation_t s_equation = {0};

static int getNumber(number_t *number, char *buf, int len)
{
    int i = 0;

    for (i = 0; i < len; i++)
    {
        number->letters[i] = strchr(s_letters.letters, buf[len - i - 1]) - s_letters.letters;

        if (i == (len - 1))
        {
            s_letters.notzero[number->letters[i]] = 1;
        }
    }
    number->num = len;
    
    return 0;
}

static int getEquation(char *buf, int len)
{
    int i = 0;
    char *ptr = NULL;

    for (i = 0; i < len; i++)
    {
        if ((buf[i] >= 'A') && (buf[i] <= 'Z'))
        {
            ptr = strchr(s_letters.letters, buf[i]);
            if (ptr == NULL)
            {
                s_letters.letters[s_letters.num] = buf[i];
                s_letters.num++;
            }
        }
    }

    ptr = strchr(buf, ' ');
    getNumber(&s_equation.x, buf, ptr - buf);

    s_equation.opt = ptr[1];

    buf = ptr + 3;
    ptr = strchr(buf, ' ');
    getNumber(&s_equation.y, buf, ptr - buf);

    buf = strchr(buf, '=') + TWO;
    ptr = buf + strlen(buf);
    while (ptr && (*ptr < 'A' || *ptr > 'Z'))
    {
        ptr--;
    }
    getNumber(&s_equation.z, buf, ptr + 1 - buf);

    return 0;
}

static int praseFile(char *path)
{
    int ret = 0;
    FILE *fp = NULL;
    char buff[512] = {0}; 
    int nread = 0;

    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    nread = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    ret = fread(buff, 1, nread, fp);
    if (ret == nread)
    {
        ret = getEquation(buff, nread);
    }

    fclose(fp);

    return ret;
}

static int generateNumber(number_t *number)
{
    int i = 0;
    int num = 0;
    
    for (i = 0; i < number->num; i++)
    {
        num += s_letters.number[number->letters[number->num - 1 - i]];
        num *= TEN;
    }
    
    return (num/TEN);
}

static void doShow(int x, int y, int z, char opt)
{
    printf("%d %c %d = %d\n", x, opt, y, z);
}

static int doCalc()
{
    int x = 0;
    int y = 0;
    int z = 0;

    x = generateNumber(&s_equation.x);
    y = generateNumber(&s_equation.y);
    z = generateNumber(&s_equation.z);

    if ((s_equation.opt == '*') && ((x * y) == z))
    {
        doShow(x,y,z,s_equation.opt);
    }

    return 0;
}

static void swap(int *a, int *b)
{
    int tmp = 0;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

static void doPermutation(int *a, int k, int m)
{
    int i = 0;

    if (k == m)
    {
        for (i = 0; i < s_letters.num; i++)
        {
            if ((s_letters.notzero[i] == 1) && 
                (s_letters.number[i] == 0))
            {
                break;
            }
        }
        
        if (i == s_letters.num)
        {
            doCalc();
        }
    }
    else
    {
        for (i = k; i <= m; i++)
        {
            swap(&a[i], &a[k]);
            doPermutation(a, k + 1, m);
            swap(&a[i], &a[k]);
        }
    }
}

static int place(int *x, int k, int total)
{
    int i = 0;
    int sum = 0;

    for (i = 0;i <= k; i++)
    {
        sum += x[i];
    }

    return sum < total;
}

static void doExtract(int *array, int total, int select)
{
    int k = 0;
    int i = 0;
    int t = 0;
    int map[MAX] = {0};

    map[k] = -1;
    while (k >= 0)
    {
        map[k]++;

        if (place(map, k, total))
        {
            if (k == (select - 1))
            {
                t = 0; 
                for (i = 0; i < select; i++)
                {
                    t += map[i];
                    s_letters.number[i] = array[t];
                }

                doPermutation(s_letters.number, 0, select-1);
            }
            else
            {
                k++;
                map[k] = 0;
            }
        }
        else
        {
            k--;
        }
    }
}

static void doProcess()
{
    int a[] = {0,1,2,3,4,5,6,7,8,9};

    doExtract(a, sizeof(a)/sizeof(a[0]), s_letters.num);
}

int main(int argc, char **argv)
{
    if ((argc != TWO) || (argv[1] == NULL))
    {
        return -1;
    }

    praseFile(argv[1]);
    
    doProcess();

    return 0;
}
