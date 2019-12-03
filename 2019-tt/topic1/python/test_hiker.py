import hiker
import unittest
import time
import os

import numpy as np


class TestHiker(unittest.TestCase):
    """
    运行入口函数，请不要修改
    如有疑问联系赛务联络人
    """

    def read_matrix_file(self, filename):
        matrix_list = []
        with open(filename, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if line is not None:
                    matrix_list.append(line.strip().split(' '))
        return np.array(matrix_list).astype(np.int32)

    def move_matrix(self, matrix, move_actions):
        if move_actions is not None and len(move_actions) > 0:
            for i in range(len(move_actions)):
                move_action = move_actions[i]
                self.check_valid(move_action)
                from_num = matrix[move_action['row_from']][move_action['col_from']]
                to_num = matrix[move_action['row_to']][move_action['col_to']]
                matrix[move_action['row_from']][move_action['col_from']] = to_num
                matrix[move_action['row_to']][move_action['col_to']] = from_num
        return matrix

    def is_line_matrix(self, matrix):
        element_locations = list(zip(np.where(matrix == 1)[0], np.where(matrix == 1)[1]))

        # 直线类型：横线
        is_transverse_line = True
        pre_location = None
        for cur_location in element_locations:
            if pre_location is None:
                pre_location = cur_location
            else:
                if pre_location[0] == cur_location[0] and cur_location[1] == pre_location[1] + 1:
                    pre_location = cur_location
                else:
                    is_transverse_line = False
                    break

        # 直线类型：竖线
        is_vertical_line = True
        pre_location = None
        for cur_location in element_locations:
            if pre_location is None:
                pre_location = cur_location
            else:
                if pre_location[0] == cur_location[0] - 1 and cur_location[1] == pre_location[1]:
                    pre_location = cur_location
                else:
                    is_vertical_line = False
                    break

        # 直线类型：向上斜线
        is_up_diagonal_line = True
        pre_location = None
        for cur_location in element_locations:
            if pre_location is None:
                pre_location = cur_location
            else:
                if pre_location[0] == cur_location[0] - 1 and cur_location[1] == pre_location[1] - 1:
                    pre_location = cur_location
                else:
                    is_up_diagonal_line = False
                    break

        # 直线类型：向下斜线
        is_down_diagonal_line = True
        pre_location = None
        for cur_location in element_locations:
            if pre_location is None:
                pre_location = cur_location
            else:
                if pre_location[0] == cur_location[0] - 1 and cur_location[1] == pre_location[1] + 1:
                    pre_location = cur_location
                else:
                    is_down_diagonal_line = False
                    break

        ret = is_transverse_line or is_vertical_line or is_up_diagonal_line or is_down_diagonal_line

        return ret

    def check_valid(self, move_action):
        if move_action['col_from'] < 0 or move_action['col_from'] >= 100 \
                or move_action['col_to'] < 0 or move_action['col_to'] >= 100 \
                or move_action['row_from'] < 0 or move_action['row_from'] >= 100 \
                or move_action['row_to'] < 0 or move_action['row_to'] >= 100:
            raise Exception('矩阵元素坐标越界。')
        d_row = abs(move_action['row_to'] - move_action['row_from'])
        d_col = abs(move_action['col_to'] - move_action['col_from'])
        if d_row > 1 or d_col > 1 or (d_row == 1 and d_col == 1):
            raise Exception('矩阵元素只能上下左右置换。')

    def test_life_the_universe_and_everything(self):
        step_count = 0
        time_total = 0
        num_of_matrixs = 1000
        for count_matrix in range(1, num_of_matrixs + 1):
            filename = os.path.join('../matrices', 'matrix') + str(count_matrix) + '.txt'
            matrix = self.read_matrix_file(filename)
            matrix_copy = matrix.copy()
            t_start = time.time()
            douglas = hiker.Hiker()
            move_actions = douglas.draw_line(matrix_copy)
            t_end = time.time()
            time_total += t_end - t_start
            # print_move_action(move_actions)

            try:
                matrix = self.move_matrix(matrix, move_actions)
                # print_matrix(matrix)
                if not self.is_line_matrix(matrix):
                    raise Exception('置换后的矩阵，所有1元素没有形成一条直线。')
                step_count += (0 if move_actions is None else len(move_actions))
                print('测试矩阵' + filename + '成功')
                print('耗费步数:' + str(step_count))
            except Exception as e:
                print('测试矩阵' + filename + '失败')
                print(e)
        print('总步数:' + str(step_count))
        print('总耗时:' + str(time_total * 1000) + 'ms')


if __name__ == '__main__':
    unittest.main()
