#pragma GCC push_options
#pragma GCC optimize ("-O1")
#include "hiker.hpp"
#include <list>
#include <vector>
#include <queue>
#include <iostream>
#include <climits>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

typedef struct {
    long x;
    long y;
} node_t;

typedef struct {
    node_t src;
    node_t dst;
} move_t;

typedef struct {
    long dist;
    node_t p1;
    node_t p2;
    list<move_t> move;
} info_t;

int all_p_on_l(node_t *p1, node_t *p2, list<node_t> *line) {
    int p_x_direct = p1->x < p2->x ? 1 : -1;
    int p_y_direct = p1->y < p2->y ? 1 : -1;

    if (p1->y == p2->y) {// y = b
        long p_x = p1->x;
        while (1) {
            if (p2->x + p_x_direct - p_x == 0)
                break;
            long p_y = p1->y;
            node_t node = {.x = p_x, .y = p_y};
            line->push_back(node);
            p_x += p_x_direct;
        }
    }
    else if (p1->x == p2->x) {// x = b
        long p_y = p1->y;
        while (1) {
            if (p2->y + p_y_direct - p_y == 0)
                break;
            long p_x = p1->x;
            node_t node = {.x = p_x, .y = p_y};
            line->push_back(node);
            p_y += p_y_direct;
        }
    }
    else if (p2->x - p1->x == p2->y - p1->y) {//y = x + b
        long p_x = p1->x;
        while (1) {
            if (p2->x + p_x_direct - p_x == 0)
                break;
            long p_y = p1->y - p1->x + p_x;
            node_t node = {.x = p_x, .y = p_y};
            line->push_back(node);
            p_x += p_x_direct;
        }
    }
    else if (p2->x - p1->x == p1->y - p2->y) {//y = -x + b
        long p_x = p1->x;
        while (1) {
            if (p2->x + p_x_direct - p_x == 0)
                break;
            long p_y = p1->y + p1->x - p_x;
            node_t node = {.x = p_x, .y = p_y};
            line->push_back(node);
            p_x += p_x_direct;
        }
    }
 
    return line->size();
}

long p2p(node_t *p1, node_t *p2) {
    return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

info_t dist_p2l(list<node_t> d_queue, node_t *p1, node_t *p2) {
    list<node_t> d_line;
    all_p_on_l(p1, p2, &d_line);
    info_t info_r;
    vector<move_t> move_r;
    info_r.dist = 0;
    info_r.p1 = *p1;
    info_r.p2 = *p2;
#if 1
loop:
    for (list<node_t>::iterator p_queue = d_queue.begin(); p_queue != d_queue.end(); p_queue++) {
        for (list<node_t>::iterator p_line = d_line.begin(); p_line != d_line.end(); p_line++) {
            if (p_queue->x == p_line->x && p_queue->y == p_line->y) {
                d_queue.erase(p_queue);
                d_line.erase(p_line);
                goto loop;
            }
        }
    }
#endif
    for (list<node_t>::iterator p_queue = d_queue.begin(); p_queue != d_queue.end();p_queue++) {
        info_t p_d_min;
        p_d_min.dist = LONG_MAX;
        list<node_t>::iterator p_remove = d_line.begin();
        for (list<node_t>::iterator p_line = d_line.begin(); p_line != d_line.end(); p_line++) {
            node_t src = *p_queue;
            node_t dst = *p_line;
            long t = p2p(&src, &dst);
            if (t < p_d_min.dist) {
                p_d_min.dist = t;
                p_d_min.p1 = src;
                p_d_min.p2 = dst;
                p_remove = p_line;
            }
        }
        d_line.erase(p_remove);
        move_t move = {.src = p_d_min.p1, .dst = p_d_min.p2};
        move_r.push_back(move);
        info_r.dist += p_d_min.dist;
    }

    float m_x = (p1->x + p2->x) / 2.0;
    while (!move_r.empty()) {
        int min_id = 0;
        float min_val = LONG_MAX;
        for (size_t i = 0; i < move_r.size(); i++) {
            float tmp = fabs(move_r[i].dst.x - m_x);
            if (tmp < min_val) {
                min_val = tmp;
                min_id = i;
            }
        }
        info_r.move.push_back(move_r[min_id]);
        move_r.erase(move_r.begin() + min_id);
    }

    return info_r;
}

vector<node_t> find_path(node_t *src, node_t *dst, int matrix[][100]) {
    vector<node_t> path;
    int visited[100][100] = {};
    node_t pathed[100][100] = {};
    int dx[] = {-1, 0, 0, 1};
    int dy[] = { 0,-1, 1, 0};

    queue<node_t> p_stack;
    p_stack.push(*src);
    visited[src->x][src->y] = 1;
    int step = 0;
    while(!p_stack.empty()){
        node_t cur = p_stack.front();
        step = visited[cur.x][cur.y] + 1;
        for (int i = 0; i < 4; i++) {
            node_t next = {.x= cur.x + dx[i], .y= cur.y + dy[i]};
            if (next.x == dst->x && next.y == dst->y) {
                pathed[next.x][next.y] = cur;
                path.push_back(next);
                for (int i = 0; i < step - 1; i++) {
                    cur = path[0];
                    path.insert(path.begin(), pathed[cur.x][cur.y]);
                }
                return path;
            }
            if (0 <= next.x && next.x <= 100-1 && 0 <= next.y && next.y <= 100-1) {
                if (matrix[next.x][next.y] == 0 and visited[next.x][next.y] == 0) {
                    visited[next.x][next.y] = step;
                    pathed[next.x][next.y] = cur;
                    p_stack.push(next);
                }
            }
        }
        p_stack.pop();
    }
    return path;
}

/**
 * 参赛选手实现该函数，使得01矩阵的所有1排成一条直线
 * @param matrix 100*100的01矩阵
 * @return 矩阵元素的置换次序
 */
list<MoveAction> DrawLine(int matrix[][100])
{
    list<node_t> p_queue;
    node_t mid_node = {.x = 0, .y = 0};

    long n_size = 100;
    for (int i = 0; i < n_size; i++) {
        for (int j = 0; j < n_size; j++) {
            if (matrix[i][j] == 1) {
                node_t node = {.x = i, .y = j};
                p_queue.push_back(node);
                mid_node.x += i;
                mid_node.y += j;
            }
        }
    }
    mid_node.x /= p_queue.size();
    mid_node.y /= p_queue.size();
    printf("%ld , %ld\n", mid_node.x, mid_node.y);

#if 1
    long x_s = mid_node.x - p_queue.size();
    x_s = x_s > 0 ? x_s : 0;
    long x_e = mid_node.x + p_queue.size() + 1;
    x_e = x_e < n_size ? x_e : n_size;
    long y_s = mid_node.y - p_queue.size();
    y_s = y_s > 0 ? y_s : 0; 
    long y_e = mid_node.y + p_queue.size() + 1;
    y_e = y_e < n_size ? y_e : n_size;
#else
    long x_s = 0;
    long x_e = 100;
    long y_s = 0;
    long y_e = 100;
#endif

    // min
    info_t d_min;
    d_min.dist = LONG_MAX;

    // y = b
    for (long x1 = x_s; x1 < x_e; x1++) {
        long x2 = x1 + p_queue.size() - 1;
        if (x2 >= n_size)
            break;
        for (long yb = y_s; yb < y_e; yb++) {
            node_t p1 = {.x = x1, .y = yb};
            node_t p2 = {.x = x2, .y = yb};
            info_t d_cur = dist_p2l(p_queue, &p1, &p2);
            /* printf("y = b dist:%ld p1:(%ld, %ld), p2:(%ld, %ld) \n", 
                d_cur.dist, 
                d_cur.p1.x, d_cur.p1.y,
                d_cur.p2.x, d_cur.p2.y); */
            d_min = d_min.dist > d_cur.dist ? d_cur : d_min;
        }
    }

    // x = b
    for (long y1 = y_s; y1 < y_e; y1++) {
        long y2 = y1 + p_queue.size() - 1;
        if (y2 >= n_size)
            break;
        for (long xb = x_s; xb < x_e; xb++) {
            node_t p1 = {.x = xb, .y = y1};
            node_t p2 = {.x = xb, .y = y2};
            info_t d_cur = dist_p2l(p_queue, &p1, &p2);
            /* printf("x = b dist:%ld p1:(%ld, %ld), p2:(%ld, %ld) \n", 
                d_cur.dist, 
                d_cur.p1.x, d_cur.p1.y,
                d_cur.p2.x, d_cur.p2.y); */
            d_min = d_min.dist > d_cur.dist ? d_cur : d_min;
        }
    }

    // y = x + b
    for (long x1 = x_s; x1 < x_e; x1++) {
        long x2 = x1 + p_queue.size() - 1;
        if (x2 >= n_size)
            break;
        for (long y1 = y_s; y1 < y_e; y1++) {
            long y2 = y1 + p_queue.size() - 1;
            if (y2 >= n_size)
                break;
            node_t p1 = {.x = x1, .y = y1};
            node_t p2 = {.x = x2, .y = y2};
            info_t d_cur = dist_p2l(p_queue, &p1, &p2);
            /* printf("y = x + b dist:%ld p1:(%ld, %ld), p2:(%ld, %ld) \n", 
                d_cur.dist, 
                d_cur.p1.x, d_cur.p1.y,
                d_cur.p2.x, d_cur.p2.y); */
            d_min = d_min.dist > d_cur.dist ? d_cur : d_min;
        }
    }

    // y = -x + b
    for (long x1 = x_s; x1 < x_e; x1++) {
        long x2 = x1 + p_queue.size() - 1;
        if (x2 >= n_size)
            break;
        for (long y2 = y_s; y2 < y_e; y2++) {
            long y1 = y2 + p_queue.size() - 1;
            if (y1 >= n_size)
                break;
            node_t p1 = {.x = x1, .y = y1};
            node_t p2 = {.x = x2, .y = y2};
            info_t d_cur = dist_p2l(p_queue, &p1, &p2);
            /* printf("y = -x + b dist:%ld p1:(%ld, %ld), p2:(%ld, %ld) \n", 
                d_cur.dist, 
                d_cur.p1.x, d_cur.p1.y,
                d_cur.p2.x, d_cur.p2.y); */
            d_min = d_min.dist > d_cur.dist ? d_cur : d_min;
        }
    }

    printf("best dist:%ld p1:(%ld, %ld), p2:(%ld, %ld) \n", 
        d_min.dist, 
        d_min.p1.x, d_min.p1.y,
        d_min.p2.x, d_min.p2.y);

    list<MoveAction> moveActions;

    for (list<move_t>::iterator p_move = d_min.move.begin(); p_move != d_min.move.end(); p_move++) {

        if (p_move->src.x == p_move->dst.x && p_move->src.y == p_move->dst.y) {
            continue;
        }

        //printf("start:(%ld, %ld)\n", p_move->src.x, p_move->src.y);
        vector<node_t> path = find_path(&p_move->src, &p_move->dst, matrix);
        if (path.size() == 0) {
            d_min.move.push_back(*p_move);
            continue;
        }
        // update
        int tmp = matrix[p_move->src.x][p_move->src.y];
        matrix[p_move->src.x][p_move->src.y] = matrix[p_move->dst.x][p_move->dst.y];
        matrix[p_move->dst.x][p_move->dst.y] = tmp;

        // move action
        for (size_t i = 0; i < path.size() - 1; i++) {
            moveActions.push_back(MoveAction(path[i].x, path[i].y, path[i+1].x, path[i+1].y));
            //printf("from:(%ld, %ld) to:(%ld, %ld)\n", path[i].x, path[i].y, path[i+1].x, path[i+1].y);
        }
        //printf("end:(%ld, %ld)\n", p_move->dst.x, p_move->dst.y);
    }

    printf("step:%lu\n", moveActions.size());
    return moveActions;
}



