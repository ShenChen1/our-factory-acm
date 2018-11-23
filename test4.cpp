#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <stdarg.h>
#include <stdlib.h>

#define MAXITEM    (1000)

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
    int depend[MAXITEM];

    int total;
} item_t;

typedef struct
{
    int item_num;
    item_t items[MAXITEM];

} safe_t;

static safe_t s_safe = {0};

static void myprintf(int level, const char *fmt, ...)
{
    va_list ap;
    
    if (level)
    {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

static int getItemNum(char *str)
{
    int ret = 0;

    ret = sscanf(str, "%d", &s_safe.item_num);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

static int getOneitemList(int item, char *str, int len)
{
    int i = 0;
    int j = 0;
    int tmp = 0;
    int flag = 0;
    
    for (i = 0; i < len; i++)
    {
        if (str[i] == ' ')
        {
            tmp = (flag) ? (-tmp) : (tmp);
            s_safe.items[item].depend[j] = tmp;
            j++;

            tmp = 0;
            flag = 0;
        }
        
        if (str[i] == '-')
        {
            flag = 1;
        }
        
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            tmp *= 10;
            tmp += (str[i] - '0');
        }
    }

    tmp = (flag) ? (-tmp) : (tmp);
    s_safe.items[item].depend[j] = tmp;
    j++;
    
    s_safe.items[item].num = j;

    return 0;
}

static int praseFile(char *path)
{
    int i = 0;
    FILE *file = NULL;
    char buff[512];

    file = fopen(path , "r");
    if (file == NULL)
    {
        return -1;
    }

    //item num
    if (NULL != fgets(buff, sizeof(buff), file))
    {
        getItemNum(buff);
    }

    while (NULL != fgets(buff, sizeof(buff), file))
    {
        getOneitemList(i, buff, strnlen(buff, sizeof(buff)));
        i++;
    }

    fclose(file);

    return 0;
}


static int doSelect(int item, int *map, int *len)
{
    int i = 0;
    int tmp = 0;
    
    *len = *len + 1;
    
    if (map[item] == 1)
    {
        return 0;
    }

    if (s_safe.items[item].total)
    {
        return s_safe.items[item].total;
    }

    for (i = 1; i < s_safe.items[item].num; i++)
    {
        map[item] = 1;
        tmp += doSelect(s_safe.items[item].depend[i] - 1, map, len);
        map[item] = 0;
    }
    
    tmp += s_safe.items[item].depend[0];

    map[item] = 1;
    s_safe.items[item].total = tmp;

    return tmp;
}


static void doProcess()
{
    int i = 0;
    int sum = 0;
    int len[MAXITEM] = {0};
    int map[MAXITEM] = {0};

    for (i = 0; i < s_safe.item_num; i++)
    {
        if (s_safe.items[i].depend[0] <= 0)
        {
            continue;
        }

        sum += doSelect(i, map, &len[i]);
    }

    myprintf(ONE, "%d\n", sum);
    for (i = 0; i < s_safe.item_num; i++)
    {
        if (map[i])
        {
            myprintf(ONE, "%d\n", i+1);
        }
    }
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
