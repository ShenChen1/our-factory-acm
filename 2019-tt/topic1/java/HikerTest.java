import org.junit.Assert;
import org.junit.Test;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class HikerTest {

    private static final int rows = 100;
    private static final int cols = 100;

    /**
     * 判断该矩阵所有1是否形成一条直线
     * @param matrix
     * @return
     */
    public static boolean IsLineMatrix( int[][] matrix )
    {
        boolean ret;

        List<ElementLocation> elementLocations = new ArrayList<ElementLocation>();

        //先找出所有1的元素
        for( int i=0; i < rows; i++ )
        {
            for( int j=0; j < cols; j++ )
            {
                if( matrix[i][j] == 1  )
                {
                    elementLocations.add(new ElementLocation(i,j));
                }
            }
        }

        //直线类型：横线
        boolean isTransverseLine = true;
        ElementLocation preLocation = null;
        ElementLocation curLocation ;
        for( int i=0; i<elementLocations.size();i++)
        {
            curLocation = elementLocations.get(i);

            if( preLocation == null )
            {
                preLocation = curLocation;
            }
            else
            {
                if( preLocation.row == curLocation.row && (curLocation.col == preLocation.col + 1) )
                {
                    preLocation = curLocation;
                }
                else
                {
                    isTransverseLine = false;
                    break;
                }
            }
        }

        //直线类型：竖线
        boolean isVerticalLine = true;
        preLocation = null;
        for( int i=0; i<elementLocations.size();i++)
        {
            curLocation = elementLocations.get(i);

            if( preLocation == null )
            {
                preLocation = curLocation;
            }
            else
            {
                if( (preLocation.row == curLocation.row - 1) && (curLocation.col == preLocation.col) )
                {
                    preLocation = curLocation;
                }
                else
                {
                    isVerticalLine = false;
                    break;
                }
            }
        }

        //直线类型：向上斜线
        boolean isUpDiagonalLine = true;
        preLocation = null;
        for( int i=0; i<elementLocations.size();i++)
        {
            curLocation = elementLocations.get(i);

            if( preLocation == null )
            {
                preLocation = curLocation;
            }
            else
            {
                if( (preLocation.row == curLocation.row - 1) && (curLocation.col == preLocation.col - 1) )
                {
                    preLocation = curLocation;
                }
                else
                {
                    isUpDiagonalLine = false;
                    break;
                }
            }
        }

        //直线类型：向下斜线
        boolean isDownDiagonalLine = true;
        preLocation = null;
        for( int i=0; i<elementLocations.size();i++)
        {
            curLocation = elementLocations.get(i);

            if( preLocation == null )
            {
                preLocation = curLocation;
            }
            else
            {
                if( (preLocation.row == curLocation.row - 1) && (curLocation.col == preLocation.col + 1) )
                {
                    preLocation = curLocation;
                }
                else
                {
                    isDownDiagonalLine = false;
                    break;
                }
            }
        }

        ret = isTransverseLine || isVerticalLine || isUpDiagonalLine || isDownDiagonalLine;

        return  ret;
    }

    /**
     * 检验是否是有效的置换动作
     * @param moveAction
     * @return
     */
    public static void CheckValid(MoveAction moveAction) throws Exception {
        if( moveAction.col_from < 0 || moveAction.col_from >= 100 ||
            moveAction.col_to < 0 || moveAction.col_to >= 100 ||
            moveAction.row_from < 0 || moveAction.row_from >= 100 ||
            moveAction.row_to < 0 || moveAction.row_to >= 100)
        {
            throw new Exception("矩阵元素坐标越界。");
        }

        int d_row = Math.abs(moveAction.row_to - moveAction.row_from);
        int d_col = Math.abs(moveAction.col_to - moveAction.col_from);
        if(  d_row > 1  || d_col > 1 ||
             (d_row == 1 && d_col == 1))
        {
            throw new Exception("矩阵元素只能上下左右置换。");
        }
    }

    /**
     * 移动矩阵元素
     * @param matrix
     * @param moveActions
     * @return
     * @throws Exception
     */
    public static int[][] MoveMatrix(int[][] matrix, List<MoveAction> moveActions) throws Exception {
        if( moveActions != null && moveActions.size() > 0)
        {
            for( int i=0; i<moveActions.size(); i++ )
            {
                MoveAction moveAction = moveActions.get(i);

                CheckValid(moveAction);

                int from = matrix[moveAction.row_from][moveAction.col_from];
                int to = matrix[moveAction.row_to][moveAction.col_to];

                matrix[moveAction.row_from][moveAction.col_from] = to;
                matrix[moveAction.row_to][moveAction.col_to] = from;
            }
        }

        return matrix;
    }

    /**
     * 读取01矩阵
     */
    public static int[][] readMatrixFile(String pathname ) {
        //String pathname = "input.txt";
        // 绝对路径或相对路径都可以，写入文件时演示相对路径,读取以上路径的input.txt文件
        //防止文件建立或读取失败，用catch捕捉错误并打印，也可以throw;
        //不关闭文件会导致资源的泄露，读写文件都同理
        //Java7的try-with-resources可以优雅关闭文件，异常时自动关闭文件；详细解读https://stackoverflow.com/a/12665271

        int[][] matrix = new int[rows][cols];

        int row = 0;

        try (FileReader reader = new FileReader(pathname);
             BufferedReader br = new BufferedReader(reader) // 建立一个对象，它把文件内容转成计算机能读懂的语言
        ) {
            String line;
            //网友推荐更加简洁的写法
            while ((line = br.readLine()) != null) {
                // 一次读入一行数据
                //System.out.println(line);
                String[] splits = line.split(" ");

                for( int col=0; col<cols; col++ )
                {
                    matrix[row][col] = Integer.parseInt(splits[col]);
                }
                row++;

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return matrix;
    }


    //测试矩阵数量
    private static final int num_of_matrixs = 1000;

    
    @Test
    public void life_the_universer_and_everything() {
	// write your code here

        //耗费步数
        int step_count = 0;
        Long time_total = 0L;

        for( int count_matrix = 1; count_matrix <= num_of_matrixs; count_matrix++ )
        {

            //读取矩阵
            String filename = "../matrices/matrix"  + count_matrix + ".txt";

            int[][] matrix = readMatrixFile(filename);

            int[][] matrix_copy = new int[rows][cols];

            //拷贝二维数组
            copy2DArray(matrix, matrix_copy);


            Long begin = System.currentTimeMillis();
            List<MoveAction> moveActions = Hiker.DrawLine(matrix_copy);
            Long end = System.currentTimeMillis();

            time_total += (end-begin);

            //printMoveAction(moveActions);

            try {

                matrix = MoveMatrix(matrix, moveActions);

                //PrintMatrix(matrix);


                if (IsLineMatrix(matrix) == false) {
                    throw new Exception("置换后的矩阵，所有1元素没有形成一条直线。");
                }

                step_count += (moveActions == null ? 0 : moveActions.size());

                System.out.println("测试矩阵"+filename+"成功");
                System.out.println("耗费步数:" + step_count);

            } catch (Exception e) {
                System.out.println("测试矩阵" + filename + "失败");
                System.out.println(e.getMessage());
            }
        }

        System.out.println("总步数:" + step_count);
        System.out.println("总耗时:" + time_total + "ms");
    }

    private static void printMoveAction(List<MoveAction> moveActions) {
        for (int i = 0; i < moveActions.size(); i++) {
            MoveAction moveAction = moveActions.get(i);
            System.out.println(
                    moveAction.row_from + "," + moveAction.col_from + " -> " +
                            moveAction.row_to + "," + moveAction.col_to);
        }
    }


    private static void copy2DArray(int[][] matrix, int[][] matrix_copy) {
        for( int i=0; i<rows; i++ )
        {
            for( int j=0; j<cols; j++ )
            {
                matrix_copy[i][j] = matrix[i][j];
            }
        }
    }

    private static void PrintMatrix(int[][] matrix) {
        for( int i=0; i< rows; i++ )
        {
            for( int j=0; j< cols; j++ )
            {
                System.out.print(matrix[i][j] + " ");
            }
            System.out.print("\r\n");
        }
    }
}
