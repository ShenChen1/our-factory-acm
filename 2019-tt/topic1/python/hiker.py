# -*- coding:utf-8 -*-

import numpy as np
import math


class Hiker:

    def draw_line(self, matrix):
        """
        参赛选手实现该函数，使得01矩阵的所有1排成一条直线
        :param matrix 100*100的01矩阵
        :return: 矩阵元素的置换次序move_actions,是形如[{'row_from': 67, 'col_from': 23, 'row_to': 68, 'col_to': 23}, ...]这样由转换dict组成的list
        """
        move_actions = []
        n_size = len(matrix)
        p_queue = [(x, y) for x in range(n_size) for y in range(n_size) if matrix[x][y]]
        p_num = len(p_queue)

        d_min = {'dist': np.inf, 'p1': (0, 0), 'p2': (0, 0), 'move': []}

        (_x, _y) = (0, 0)
        for (x,y) in p_queue:
            _x += x
            _y += y
        _x = _x // p_num
        _y = _y // p_num
        print((_x, _y))

        x_s = _x - p_num
        x_s = x_s if x_s > 0 else 0 
        x_e = _x + p_num + 1
        x_e = x_e if x_e < n_size else n_size
        y_s = _y - p_num
        y_s = y_s if y_s > 0 else 0 
        y_e = _y + p_num + 1
        y_e = y_e if y_e < n_size else n_size

        # y = b
        for x1 in range(x_s, x_e):
            x2 = x1 + p_num - 1
            if x2 >= n_size:
                break
            for yb in range(y_s, y_e):
                d_cur = self.dist_p2l(p_queue, (x1, yb), (x2, yb))
                # print("y = b", d_min['dist'], d_cur)
                d_min = d_cur if d_min['dist'] > d_cur['dist'] else d_min

        # x = b
        for y1 in range(y_s, y_e):
            y2 = y1 + p_num - 1
            if y2 >= n_size:
                break
            for xb in range(x_s, x_e):
                d_cur = self.dist_p2l(p_queue, (xb, y1), (xb, y2))
                # print("x = b", d_min['dist'], d_cur)
                d_min = d_cur if d_min['dist'] > d_cur['dist'] else d_min

        # y = x + b
        for x1 in range(x_s, x_e):
            x2 = x1 + p_num - 1
            if x2 >= n_size:
                break
            for y1 in range(y_s, y_e):
                y2 = y1 + p_num - 1
                if y2 >= n_size:
                    break
                d_cur = self.dist_p2l(p_queue, (x1, y1), (x2, y2))
                # print("y = x + b", d_min['dist'], d_cur)
                d_min = d_cur if d_min['dist'] > d_cur['dist'] else d_min

        # y = -x + b
        for x1 in range(x_s, x_e):
            x2 = x1 + p_num - 1
            if x2 >= n_size:
                break
            for y1 in range(y_s, y_e):
                y2 = y1 - p_num + 1
                if y2 < 0:
                    continue
                d_cur = self.dist_p2l(p_queue, (x1, y1), (x2, y2))
                # print("y = -x + b", d_min['dist'], d_cur)
                d_min = d_cur if d_min['dist'] > d_cur['dist'] else d_min

        # point to point
        print("best", d_min)
        d_min = self.dist_p2l_f(p_queue, d_min['p1'], d_min['p2'])
        for (src, dst) in d_min['move']:
            # print("start:", src)
            path = self.find_path(src, dst, matrix)
            # print("path", path)
            if path[len(path) - 1] != dst:
                d_min['move'].append((src, dst))
                continue
            # update
            (src_x, src_y) = src
            (dst_x, dst_y) = dst
            tmp = matrix[src_x][src_y]
            matrix[src_x][src_y] = matrix[dst_x][dst_y]
            matrix[dst_x][dst_y] = tmp
            # move action
            for i in range(len(path) - 1):
                (src_x, src_y) = path[i]
                (dst_x, dst_y) = path[i + 1]
                move_actions.append({'row_from': src_x, 'col_from': src_y, 'row_to': dst_x, 'col_to': dst_y})
            # print("end:", dst)

        print("step", len(move_actions))
        return move_actions

    def dist_p2l(self, p_queue, p1, p2):
        d_cur = {'dist': 0, 'p1': p1, 'p2': p2}
        for p in p_queue:
            d_cur['dist'] += self.p2p(p, self.p2seg(p, p1, p2))

        return d_cur

    def dist_p2l_f(self, p_queue, p1, p2):
        d_cur = {'dist': 0, 'p1': p1, 'p2': p2, 'move': []}
        d_line = self.all_p_on_l(p1, p2)
        d_queue = p_queue.copy()
        d_queue_t = p_queue.copy()
        for p in d_queue_t:
            if p in d_line:
                d_line.remove(p)
                d_queue.remove(p)

        for src in d_queue:
            p_d_min = {'dist':np.inf}
            for dst in d_line:
                p_d_cur = self.p2p(src, dst)
                if p_d_cur < p_d_min['dist']:
                    p_d_min['dist'] = p_d_cur
                    p_d_min['src'] = src
                    p_d_min['dst'] = dst
            d_line.remove(p_d_min['dst'])
            d_cur['move'].append((p_d_min['src'], p_d_min['dst']))
            d_cur['dist'] += p_d_min['dist']

        return d_cur

    def all_p_on_l(self, p1, p2):
        line = []
        (p1_x, p1_y) = p1
        (p2_x, p2_y) = p2
        p_x_direct = 1 if p1_x < p2_x else -1
        p_y_direct = 1 if p1_y < p2_y else -1

        if p1_y == p2_y: # y = b
            for p_x in range(p1_x, p2_x + p_x_direct, p_x_direct):
                p_y = p1_y
                line.append((p_x, p_y))
        elif p1_x == p2_x: # x = b
            for p_y in range(p1_y, p2_y + p_y_direct, p_y_direct):
                p_x = p1_x
                line.append((p_x, p_y))
        elif p2_x - p1_x == p2_y - p1_y: # y = x + b
            for p_x in range(p1_x, p2_x + p_x_direct, p_x_direct):
                p_y = p1_y - p1_x + p_x
                line.append((p_x, p_y))
        elif p2_x - p1_x == p1_y - p2_y: # y = -x + b
            for p_x in range(p1_x, p2_x + p_x_direct, p_x_direct):
                p_y = p1_y + p1_x - p_x
                line.append((p_x, p_y))

        # print(">>line", line)
        return line

    def p2seg(self, p, p1, p2):
        (x,y) = p
        (x1, y1) = p1
        (x2, y2) = p2
        cross = (x2 - x1) * (x - x1) + (y2 - y1) * (y - y1);
        if cross <= 0:
            return (x1, y1)

        d2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
        if cross >= d2:
            return (x2, y2)

        r = cross / d2;
        px = x1 + (x2 - x1) * r;
        py = y1 + (y2 - y1) * r;

        return (int(px), int(py))

    def p2p(self, p1, p2):
        (x1, y1) = p1
        (x2, y2) = p2
        # return math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))
        return abs(x1 - x2) + abs(y1 - y2)

    def find_path(self, p1, p2, grid):
        n, m = len(grid), len(grid[0])
        p1_x, p1_y = p1
        p2_x, p2_y = p2
        p_x_direct = 1 if p1_x < p2_x else -1
        p_y_direct = 1 if p1_y < p2_y else -1

        trytime = 0
        path = [p1]
        cur_x = p1_x
        cur_y = p1_y

        while True:
            for x in range(cur_x + p_x_direct, p2_x + p_x_direct, p_x_direct):
                    if grid[x][cur_y] == 1:
                        break
                    path.append((x, cur_y))
                    cur_x = x
            for y in range(cur_y + p_y_direct, p2_y + p_y_direct, p_y_direct):  
                    if grid[cur_x][y] == 1:
                        break
                    path.append((cur_x, y))
                    cur_y = y
            trytime += 1
            if trytime == 1:
                break

        if cur_x != p2_x or cur_y != p2_y:
            path += self.__find_path((cur_x, cur_y), p2, grid)

        return path

    def __find_path(self, src, dst, grid):
        n, m = len(grid), len(grid[0])
        src_x, src_y = src
        dst_x, dst_y = dst
        visited = [[0] * m for _ in range(n)]
        pathed = [[(0,0)] * m for _ in range(n)]

        x = [-1, 0, 0, 1]
        y = [ 0,-1, 1, 0]
        stack =[(src_x, src_y)]
        visited[src_x][src_y] = 1
        step = 0
        temp_list = []
        while len(stack):
            cur_x, cur_y = stack.pop()
            step = visited[cur_x][cur_y]
            step += 1
            for i in range(4):
                next_x, next_y = cur_x + x[i], cur_y + y[i]
                if next_x == dst_x and next_y == dst_y:
                    pathed[next_x][next_y] = (cur_x, cur_y)
                    path = [(next_x, next_y)]
                    for i in range(0, step - 1):
                        (x, y) = path[0]
                        path.insert(0, pathed[x][y])
                    return path
                if 0 <= next_x <= n-1 and 0 <= next_y <= m-1 and grid[next_x][next_y] == 0 and visited[next_x][next_y] == 0:
                    temp_list.append((next_x, next_y))
                    visited[next_x][next_y] = step
                    pathed[next_x][next_y] = (cur_x, cur_y)
            if len(stack) == 0:
                stack = temp_list.copy()
                temp_list = []
        return []
