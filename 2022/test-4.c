

#pragma GCC push_options
#pragma GCC optimize("-O3")
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    int s;
    int e;
    int c;
} edge_t;

int g_n_nodes = 0;
int g_n_edges = 0;
int **g_graph = NULL;
edge_t *g_edges = NULL;
int *g_fa = NULL;

int cmp(const void *a, const void *b)
{
    const edge_t *x = a;
    const edge_t *y = b;

    return x->c > y->c;
}

int find(int *fa, int x)
{
    if (x != fa[x]) {
        fa[x] = find(fa, fa[x]);
    }
    return fa[x];
}

long long kruscal(const edge_t *edges, int n_edges, int *fa)
{
    int i;
    long long total = 0;
    int num = 0;
    for (i = 0; i < n_edges; i++) {
        int t1 = find(fa, edges[i].s);
        int t2 = find(fa, edges[i].e);
        if (t1 != t2) {
            fa[t1] = t2;
            total += edges[i].c;
#if DEBUG
            printf("kruscal:% lld %d\n", total, edges[i].c);
#endif
            num += 1;
            if (num == n_edges) {
                break;
            }
        }
    }
    return total;
}

int result(long long a, long long b)
{
    static char cmdline[256] = {};
    long long m = 1000000007;

    sprintf(cmdline, "python3.8 -c \"print((%lld * pow(%lld, -1, mod=%lld))%%%lld)\"", a, b, m, m);
    system(cmdline);
    return 0;
}

int run()
{
    int i, j, k;
    long long ans = 0;
    long long time = 0;

    for (i = 1; i <= g_n_nodes; i++) {
        for (j = 1; j < i; j++) {
            edge_t save = {-1, -1, -1};
            int index = g_graph[i][j];
            if (index) {
                save = g_edges[index - 1];
                g_edges[index - 1].c = 0;
            } else {
                g_edges[g_n_edges].s = i;
                g_edges[g_n_edges].e = j;
                g_edges[g_n_edges].c = 0;
                g_n_edges += 1;
            }
            qsort(g_edges, g_n_edges, sizeof(edge_t), cmp);
#if DEBUG
            for (k = 0; k < g_n_edges; k++) {
                printf("s[%d][%d]: s:%d e:%d c:%d\n", i, j, g_edges[k].s, g_edges[k].e, g_edges[k].c);
            }
#endif
            for (k = 0; k <= g_n_nodes; k++) {
                g_fa[k] = k;
            }

            ans += kruscal(g_edges, g_n_edges, g_fa);
            time += 1;

            if (index) {
                g_edges[0] = save;
            } else {
                g_n_edges -= 1;
                memcpy(g_edges, g_edges + 1, sizeof(edge_t) * g_n_edges);
            }

            qsort(g_edges, g_n_edges, sizeof(edge_t), cmp);
#if DEBUG
            for (k = 0; k < g_n_edges; k++) {
                printf("e[%d][%d]: s:%d e:%d c:%d\n", i, j, g_edges[k].s, g_edges[k].e, g_edges[k].c);
            }
#endif
        }
    }

    return result(ans, time);
}

long long solve(int N, int M, int **A)
{
    int i;
    g_n_nodes = N;
    g_edges = calloc(M + 1, sizeof(edge_t));
    g_graph = calloc((g_n_nodes + 1), sizeof(int *));
    for (i = 1; i <= g_n_nodes; i++) {
        g_graph[i] = calloc((i + 1), sizeof(int));
    }

    for (i = 0; i < M; i++) {
        int start = A[i][0];
        int end = A[i][1];
        int cost = A[i][2];
        free(A[i]);

        if (start < end) {
            int tmp = start;
            start = end;
            end = tmp;
        }

        if (g_graph[start][end]) {
            int index = g_graph[start][end] - 1;
            g_edges[index].c = MIN(cost, g_edges[index].c);
            continue;
        }

        g_graph[start][end] = g_n_edges + 1;
        g_edges[g_n_edges].s = start;
        g_edges[g_n_edges].e = end;
        g_edges[g_n_edges].c = cost;
        g_n_edges += 1;
    }
    free(A);
    g_fa = malloc(sizeof(int) * (g_n_nodes + 1));

    qsort(g_edges, g_n_edges, sizeof(edge_t), cmp);
    for (i = 0; i < g_n_edges; i++) {
        int start = g_edges[i].s;
        int end = g_edges[i].e;
        g_graph[start][end] = i + 1;
        // printf("[%d]: s:%d e:%d c:%d\n", i, start, end, g_edges[i].c);
    }

    return run();
}

int main()
{
    int N;
    scanf("%d", &N);
    int M;
    scanf("%d", &M);
    int i_A;
    int **A = (int **)malloc((M) * sizeof(int *));
    for (i_A = 0; i_A < M; i_A++) {
        A[i_A] = (int *)malloc((3) * sizeof(int));
    }
    for (i_A = 0; i_A < M; i_A++) {
        scanf("%d %d %d", &A[i_A][0], &A[i_A][1], &A[i_A][2]);
    }

    return solve(N, M, A);
}
