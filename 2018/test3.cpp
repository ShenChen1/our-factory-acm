#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <stdarg.h>
#include <stdlib.h>

#define MAXUSER    (10)
#define MAXLINE    (20)

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
    int list[MAXLINE];
} user_t;

typedef struct
{
    int user_num;
    user_t usrs[MAXUSER];

    int line_num;
    int lines[MAXLINE];

} testline_t;

static testline_t s_testline = {0};

static int getUsersAndLines(char *str)
{
    int i = 0;
    int ret = 0;

    ret = sscanf(str, "%d, %d", &s_testline.line_num, &s_testline.user_num);
    if (ret < 0)
    {
        return -1;
    }

    for (i = 0; i < s_testline.line_num; i++)
    {
        s_testline.lines[i] = i;
    }
    
    for (i = 0; i < s_testline.user_num; i++)
    {
        s_testline.usrs[i].num = 0;
    }

    return 0;
}

static int getOneuserList(int user, char *str, int len)
{
    int i = 0;
    int j = 0;
    int tmp = 0;
    
    for (i = 0; i < len; i++)
    {
        if (str[i] == ',')
        {
            s_testline.usrs[user].list[j] = tmp - 1;
            j++;

            tmp = 0;
        }
        
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            tmp *= TEN;
            tmp += (str[i] - '0');
        }
    }

    s_testline.usrs[user].list[j] = tmp - 1;
    j++;
    
    s_testline.usrs[user].num = j;

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

    //users_and_lines
    if (NULL != fgets(buff, sizeof(buff), file))
    {
        getUsersAndLines(buff);
    }

    while (NULL != fgets(buff, sizeof(buff), file))
    {
        getOneuserList(i, buff, strnlen(buff, sizeof(buff)));
        i++;
    }

    fclose(file);

    return 0;
}


static int doExtract(int user, int *map)
{
    int i = 0;
    int tmp = 0;
    int sum = 0;

    if (user == s_testline.user_num)
    {
        return 1;
    }
    
    for (i = 0; i < s_testline.usrs[user].num; i++)
    {
        tmp = s_testline.usrs[user].list[i];
        if (map[tmp] == 0)
        {
            map[tmp] = 1;
            sum += doExtract(user + 1, map);
            map[tmp] = 0;
        }
    }
    
    return sum;
}

static void doProcess()
{
    int map[MAXLINE] = {0};

    fprintf(stdout, "%d\n", doExtract(0, map));
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
