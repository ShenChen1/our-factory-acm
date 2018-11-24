#pragma GCC push_options
#pragma GCC optimize ("-O3")
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
#include <string>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;
typedef long long LL;

#define MAXITEM     (1000+16)
#define MAXCAP      (0x3f3f3f3f3f3f3f3f)

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
    LL num;
    LL depend[MAXITEM];
} item_t;

typedef struct
{
    LL item_num;
    item_t items[MAXITEM];

    LL total;

} safe_t;

static safe_t s_safe = {0};

static void myprintf(LL level, const char *fmt, ...)
{
    va_list ap;
    
    if (level)
    {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

static LL getItemNum(char *str)
{
    LL ret = 0;

    ret = sscanf(str, "%lld", &s_safe.item_num);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

static LL getOneitemList(LL item, char *str, LL len)
{
    LL i = 0;
    LL j = 0;
    LL tmp = 0;
    LL flag = 0;
    
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

static LL praseFile(char *path)
{
    LL i = 0;
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
 
// 用于表示边的结构体（终点、容量、反向边）
class edge
{
public:
    LL to;
    LL cap;
    LL rev;
    edge(LL to, LL cap, LL rev) :to(to), cap(cap), rev(rev){}
};

 
static vector<edge> G[MAXITEM];  // 图的邻接表表示
static LL level[MAXITEM];       // 顶点到源点的距离标号
static LL iter[MAXITEM];        // 当前弧，在其之前的边已经没有用了
 
// 向图中加入一条从from到to的容量为cap的边
static void addEdge(LL from, LL to, LL cap)
{
    G[from].push_back(edge(to, cap, G[to].size() ));
    G[to].push_back(edge(from, 0, G[from].size() - 1));
}
 
// 通过BFS计算从源点出发的距离标号
static void bfs(LL s)
{
    LL i;

    memset(level, -1, sizeof(level));
    queue<LL> que;
    level[s] = 0;
    que.push(s);
    while (!que.empty())
    {
        LL v = que.front(); 
        que.pop();
        for (i = 0; i < static_cast<LL>(G[v].size()); ++i)
        {
            edge& e = G[v][i];
            if (e.cap > 0 && level[e.to] < 0)
            {
                level[e.to] = level[v] + 1;
                que.push(e.to);
            }
        }
    }
}
 
// 通过DFS寻找增广路
static LL dfs(LL v, LL t, LL f)
{
    if (v == t)
    {
        return f;
    }

    for (LL& i = iter[v]; i < static_cast<LL>(G[v].size()); ++i)
    {
        edge& e = G[v][i];
        if (e.cap > 0 && level[v] < level[e.to])
        {
            LL d = dfs(e.to, t, min(f, e.cap));
            if (d > 0)
            {
                e.cap -= d;
                G[e.to][e.rev].cap += d;
                return d;
            }
        }
    }
 
    return 0;
}
 
// 求解从s到t的最大流
static LL maxFlow(LL s, LL t)
{
    LL flow = 0;
    for (;;)
    {
        bfs(s);
        if (level[t] < 0) 
        {
            return flow;
        }
        memset(iter, 0, sizeof(iter));
        LL f;
        while ((f = dfs(s, t, MAXCAP)) > 0)
        {
            flow += f;
        }
    }
}
 
static LL visited[MAXITEM];
// 遍历残余网络
static void solve(LL v)
{
    LL i;

    visited[v] = true;
    for (i = 0; i < static_cast<LL>(G[v].size()); ++i) 
    {
        const edge &e = G[v][i];
        if (e.cap > 0 && !visited[e.to]) 
        {
            solve(e.to);
        }
    }
}


static void doProcess()
{
    LL i;
    LL j;
    const LL s = 0;
    const LL t = s_safe.item_num + 1;

    for (i = 0; i < s_safe.item_num; i++)
    {
        LL w = s_safe.items[i].depend[0];
        if (w > 0)
        {
            s_safe.total += w;
            addEdge(s, i+1, w);
        }

        if (w < 0)
        {
            addEdge(i+1, t, -w);
        }
    }

    for (i = 0; i < s_safe.item_num; ++i)
    {
        for (j = 1; j < s_safe.items[i].num; j++)
        {
            addEdge(i+1, s_safe.items[i].depend[j], MAXCAP);
        }
    }
    
    LL maxProfit = s_safe.total - maxFlow(s, t);
    solve(s);

    myprintf(ONE, "%lld\n", maxProfit);
    for (i = 1; i <= s_safe.item_num; i++)
    {
        if (visited[i])
        {
            myprintf(ONE, "%lld\n", i);
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
