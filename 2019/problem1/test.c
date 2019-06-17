#include <stdio.h>

typedef enum {
    ZERO = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
} number_t;

typedef struct {
    int v1;
    int v2;
} arc_t;

#define DEBUG_SOLVE

#define MAX_VERtEX_NUM 100                  //顶点的最大个数
#define VRType int                          //表示弧的权值的类型
#define VertexType int                      //图中顶点的数据类型
#define INFINITY 65535
typedef struct {
    VertexType vexs[MAX_VERtEX_NUM];        //存储图中顶点数据
    VRType arcs[MAX_VERtEX_NUM][MAX_VERtEX_NUM];                //二维数组，记录顶点之间的关系
    int vexnum, arcnum;                      //记录图的顶点数和弧（边）数
} MGraph;
typedef int PathMatrix[MAX_VERtEX_NUM];     //用于存储最短路径中经过的顶点的下标
typedef int ShortPathTable[MAX_VERtEX_NUM]; //用于存储各个最短路径的权值和
//根据顶点本身数据，判断出顶点在二维数组中的位置
int LocateVex(MGraph *G, VertexType v) {
    int i;
    //遍历一维数组，找到变量v
    for (i = 0; i < G->vexnum; i++) {
        if (G->vexs[i] == v) {
            printf("find\n");
            break;
        }
    }
    //如果找不到，输出提示语句，返回-1
    if (i == G->vexnum) {
        printf("no such vertex.\n");
        return -1;
    }
    return i;
}
//构造有向网
void CreateUDG(MGraph *G, char **tmpVex, int vexnum, arc_t *tmpArc, int arcnum) {
    int i, j;

    G->vexnum = vexnum;
    G->arcnum = arcnum;
    for (i = 0; i < G->vexnum; i++) {
        G->vexs[i] = i;
    }
    for (i = 0; i < G->vexnum; i++) {
        for (j = 0; j < G->vexnum; j++) {
            G->arcs[i][j] = INFINITY;
        }
    }
    for (i = 0; i < G->arcnum; i++) {
        int v1, v2, w;
        v1 = tmpArc[i].v1;
        v1 = tmpArc[i].v2;
        w = 1;
        int n = LocateVex(G, v1);
        int m = LocateVex(G, v2);
        if (m == -1 || n == -1) {
            printf("no this vertex\n");
            return;
        }
        G->arcs[n][m] = w;
    }
}
//迪杰斯特拉算法，v0表示有向网中起始点所在数组中的下标
void ShortestPath_Dijkstra(MGraph G, int v0, PathMatrix *p, ShortPathTable *D) {
    int v, i, k, w;

    int final[MAX_VERtEX_NUM]; //用于存储各顶点是否已经确定最短路径的数组
    //对各数组进行初始化
    for (v = 0; v < G.vexnum; v++) {
        final[v] = 0;
        (*D)[v] = G.arcs[v0][v];
        (*p)[v] = 0;
    }
    //由于以v0位下标的顶点为起始点，所以不用再判断
    (*D)[v0] = 0;
    final[v0] = 1;
    for (i = 0, k = 0; i < G.vexnum; i++) {
        int min = INFINITY;
        //选择到各顶点权值最小的顶点，即为本次能确定最短路径的顶点
        for (w = 0; w < G.vexnum; w++) {
            if (!final[w]) {
                printf("(*D)[w]:%d\n", (*D)[w]);
                if ((*D)[w] < min) {
                    k = w;
                    min = (*D)[w];
                }
            }
        }
        //设置该顶点的标志位为1，避免下次重复判断
        final[k] = 1;
        //对v0到各顶点的权值进行更新
        for (w = 0; w < G.vexnum; w++) {
            printf("G.arcs[k][w]:%d\n", G.arcs[k][w]);
            if (!final[w] && (min + G.arcs[k][w] < (*D)[w])) {
                (*D)[w] = min + G.arcs[k][w];
                (*p)[w] = k;    //记录各个最短路径上存在的顶点
            }
        }
    }
}

static void showArray(char **array, int size) {

    int i;

    printf("[");
    for (i = 0; i < size; i++) {
        printf("%s ", array[i]);
    }
    printf("]\n");
}

static void showMap(char **path, int *map, int x, int y) {

    int i, j;

    printf("%20s ", " ");
    for (i = 0; i < y; i++) {
        printf("%20s ", path[i]);
    }
    printf("\n");

    for (j = 0; j < x; j++) {

        printf("%20s ", path[j]);

        for (i = 0; i < y; i++) {

            printf("%20d", map[j * x + i]);

        }

        printf("\n");
    }
}

static int getSolvingPath(char **transferPath, int transferSize,
        char ***solvingPath, int *solvingSize) {
    int i, j, k;
    int tmpSize = ZERO;
    char **tmpArray = NULL;
    char **tmpVex = NULL;
    arc_t *tmpArc = NULL;

    tmpArray = (char **) malloc(sizeof(char *) * transferSize);
    if (tmpArray == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    tmpVex = (char **) malloc(sizeof(char *) * transferSize);
    if (tmpVex == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    tmpArc = (arc_t *) malloc(sizeof(arc_t) * transferSize);
    if (tmpArc == NULL) {
        fprintf(stderr, "malloc err\n");
        return -ONE;
    }

    //获取vex
    int vexnum = 0;
    for (i = 0; i < transferSize; i++) {

        for (j = 0; j < vexnum; j++) {

            if (strcmp(transferPath[i], tmpVex[j]) == ZERO) {
                break;
            }
        }

        if (j == vexnum) {
            tmpVex[vexnum] = transferPath[i];
            vexnum++;
        }
    }

    for (i = 0; i < vexnum; i++) {
        printf("[%d]%s\n", i, tmpVex[i]);
    }

    //获取arc
    int arcnum = 0;
    for (i = 0; i < transferSize - 1; i++) {

        int v1 = vexnum;
        int v2 = vexnum;
        for (j = 0; j < vexnum; j++) {
            if (strcmp(tmpVex[j], transferPath[i]) == ZERO) {
                v1 = j;
                break;
            }
        }
        for (j = 0; j < vexnum; j++) {
            if (strcmp(tmpVex[j], transferPath[i + 1]) == ZERO) {
                v2 = j;
                break;
            }
        }

        for (j = 0; j < arcnum; j++) {

            if (v1 == tmpArc[j].v1 && v2 == tmpArc[j].v2) {
                break;
            }
        }

        if (j == arcnum) {
            tmpArc[arcnum].v1 = v1;
            tmpArc[arcnum].v2 = v2;
            arcnum++;
        }
    }

    for (i = 0; i < arcnum; i++) {
        printf("[%d]  [%d]%s [%d]%s\n", i, tmpArc[i].v1, tmpVex[tmpArc[i].v1],
                tmpArc[i].v2, tmpVex[tmpArc[i].v2]);
    }

    MGraph G;
    CreateUDG(&G, tmpVex, vexnum, tmpArc, arcnum);
    PathMatrix P;
    ShortPathTable D;
    ShortestPath_Dijkstra(G, 0, &P, &D);
    for (i = 1; i < G.vexnum; i++) {
        printf("V%d - V%d的最短路径中的顶点有(逆序)：", 0, i);
        printf(" V%d", i);
        j = i;
        //由于每一段最短路径上都记录着经过的顶点，所以采用嵌套的方式输出即可得到各个最短路径上的所有顶点
        while (P[j] != 0) {
            printf(" V%d", P[j]);
            j = P[j];
        }
        printf(" V0\n");
    }
    printf("源点到各顶点的最短路径长度为:\n");
    for (i = 1; i < G.vexnum; i++) {
        printf("V%d - V%d : %d \n", G.vexs[0], G.vexs[i], D[i]);
    }

    *solvingPath = tmpArray;
    *solvingSize = tmpSize;

    return ZERO;
}

int main() {
#if 0
    char *transferPath[] = {
        "RABLTESWPWRMACPS",
        "NIHZSIV5",
        "NIULSD1",
        "LTE_DEVCAHZ_CHZ_UPMACPS",
        "LTE_DEVLN1_FRLN_L2",
        "RCPDEV",
        "RCPMSG",
        "NIULSD1",
        "EXT_LTE_ASB_CSH_CP",
        "NIULSD1",
        "EXT_LTE_ASB_CSH_UPMACPS",
        "RCPDEV",
        "RCPMSG",
        "RCPTNS",
        "RCPDPM",
        "RCPASM_CCS",
        "RCPDPM",
        "RCPASM_CCS",
    };
#endif

#if 1
    char *transferPath[] = { "NIULSLFS", "NIOSHCR4", "NIESSCBTSMZOAM",
            "NIOSHCR4", "NIOSDK", "NIOSHCR4", "NIULSLFS", "NIESSCBTSMZOAM",
            "NIULSLFS", "NIESSCBTSMZOAM", "NIOSHCR4", "NIOSDK", "NIOSHCR4",
            "NIESSCBTSMZOAM", };

#endif

#if 0
    char *transferPath[] = {
        "ECE_DEV_IBA_CP_RT",
        "ECE_DEV_FOU_OAM_MZ",
        "MANO_HZH_MZOM",
        "ECE_DEV_FOU_OAM_MZ_EXT",
    };
#endif

#if 0
    char *transferPath[] = {
        "ECE_DEV_FOU",
        "ECE_DEV_FOU_OAM_MZ_EXT",
        "ECE_DEV_FOU_OAM_MZ",
    };
#endif

#if 0
    char *transferPath[] = {
        "1234",
//"MANO_HZH_MZOM",
        "ECE_DEV_FOU",
        "ECE_DEV_FOU_OAM_MZ_EXT",
        "ECE_DEV_FOU_OAM_MZ",
        "2345"
    };
#endif

#if 0
    char *transferPath[] = {
        "ECE_DEV_FOU",
        "MANO_HZH_MZOM",
        "TRS_5G_ASIK_GENERAL",
        "NIOSDX",
        "NIPSSHC5G",
        "NIOSDX",
        "NIPSSHC5G",
        "NIOYSE2",
        "NIWRYSNE",
        "NIOYSE2",
        "NIULSLFS",
    };
#endif

    int i;
    char **transferGroupPath = NULL;
    int transferGroupSize = 0;

    getSolvingPath(transferPath, sizeof(transferPath) / sizeof(char *),
            &transferGroupPath, &transferGroupSize);

    for (i = 0; i < transferGroupSize; i++) {
        printf("%s\n", transferGroupPath[i]);
    }

}
