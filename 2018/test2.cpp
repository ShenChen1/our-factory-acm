#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <cstring>
#include <string>
#include <stdlib.h>

#define NOEDGE   (-1)
#define MAXM     (100)
#define MAXN     (100)

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

typedef enum
{
    MV_RIGHT = 0,
    MV_DOWN,
    MV_MAX,
} MV_ACT;

typedef struct
{
    int index;
    int value;
} node_t;

typedef struct
{
    node_t node[MV_MAX];
    int total;
} elem_t;

typedef struct
{
    int m;
    int n;

    int point[MAXM*MAXN];

    elem_t map[MAXM*MAXN];

} map_t;

static map_t s_map = {0};

static int getMAndN(char *str)
{
    int ret = 0;

    ret = sscanf(str, "%d, %d", &s_map.m, &s_map.n);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

static int getNlinePoint(int n, char *str, int len)
{
    int i = 0;
    int j = 0;
    int tmp = 0;
    
    for (i = 0; i < len; i++)
    {
        if (str[i] == ',')
        {
            s_map.point[n * s_map.m + j] = tmp;
            j++;

            tmp = 0;
        }
        
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            tmp *= TEN;
            tmp += (str[i] - '0');
        }
    }

    s_map.point[n * s_map.m + j] = tmp;
    j++;

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

    //m and n
    if (NULL != fgets(buff, sizeof(buff), file))
    {
        getMAndN(buff);
    }

    while (NULL != fgets(buff, sizeof(buff), file))
    {
        getNlinePoint(i, buff, strnlen(buff, sizeof(buff)));
        i++;
    }

    fclose(file);

    return 0;
}


static int generateMap()
{
    int i = 0;
    int j = 0;

    for (j = 0; j < s_map.n; j++)
    {
        for (i = 0; i < s_map.m; i++)
        {
            s_map.map[j * s_map.m + i].node[MV_RIGHT].value = NOEDGE;
            s_map.map[j * s_map.m + i].node[MV_DOWN].value = NOEDGE;
            s_map.map[j * s_map.m + i].total = 0;

            if ((i == (s_map.m - 1)) && (j == (s_map.n - 1)))
            {
                s_map.map[j * s_map.m + i].total = 0;
            }
            else if (i == (s_map.m - 1))
            {
                s_map.map[j * s_map.m + i].node[MV_DOWN].index = (j + 1) * s_map.m + i;
                s_map.map[j * s_map.m + i].node[MV_DOWN].value = s_map.point[(j + 1) * s_map.m + i];
                
            }
            else if (j == (s_map.n - 1))
            {
                s_map.map[j * s_map.m + i].node[MV_RIGHT].index = j * s_map.m + (i + 1);
                s_map.map[j * s_map.m + i].node[MV_RIGHT].value = s_map.point[j * s_map.m + (i + 1)];
            }
            else
            {
                s_map.map[j * s_map.m + i].node[MV_DOWN].index = (j + 1) * s_map.m + i;
                s_map.map[j * s_map.m + i].node[MV_DOWN].value = s_map.point[(j + 1) * s_map.m + i];

                s_map.map[j * s_map.m + i].node[MV_RIGHT].index = j * s_map.m + (i + 1);
                s_map.map[j * s_map.m + i].node[MV_RIGHT].value = s_map.point[j * s_map.m + (i + 1)];
            }
        }
    }

    return 0;
}

static int doCalc(int node, int sum)
{
    int i;
    int tmp = sum;
    int max = sum;

    if (node == s_map.m * s_map.n - 1)
    {
        return sum;
    }
    
    if (s_map.map[node].total)
    {
        return (s_map.map[node].total + sum);
    }

    for (i = 0; i < MV_MAX; i++)
    {
        if (s_map.map[node].node[i].value != NOEDGE)
        {
            tmp = doCalc(s_map.map[node].node[i].index, sum + s_map.map[node].node[i].value);

            max = (tmp > max) ? (tmp) : (max);
        }
    }
    
    s_map.map[node].total = max - sum;
    
    return max;
}

static void doProcess()
{
    generateMap();

    fprintf(stdout, "%d\n", doCalc(0, s_map.point[0]));
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
