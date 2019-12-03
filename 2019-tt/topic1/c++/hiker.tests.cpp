#include "hiker.hpp"
#include <cassert>

#include <time.h>
#include <iostream>
#include <sstream>
#include <fstream>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include<windows.h>
#include<winnt.h>	
#else
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include <fcntl.h>
#include <list>
#include <string> 

using namespace std; 

	
	
	/*
	**拷贝数组 
	*/ 
	void copy2DArray(int matrix[][100], int matrix_copy[][100]) {
        for( int i=0; i<100; i++ )
        {
            for( int j=0; j<100; j++ )
            {
                matrix_copy[i][j] = matrix[i][j];
            }
        }
    }
	
	/**
     * 读取01矩阵
     */
    void readMatrixFile(int matrix[][100], string pathname ) {        
        int number;
        
        std::ifstream ifile(pathname);
        
        for( int i=0; i < 100; i++ )
        {
        	for( int j=0; j<100;j++ )
        	{
        		ifile >> number;
        		matrix[i][j] = number;
			}
		}
		
		ifile.close();
    }
    
    /**
     * 检验是否是有效的置换动作
     * @param moveAction
     * @return
     */
    void CheckValid(MoveAction& moveAction) {
        if( moveAction.col_from < 0 || moveAction.col_from >= 100 ||
            moveAction.col_to < 0 || moveAction.col_to >= 100 ||
            moveAction.row_from < 0 || moveAction.row_from >= 100 ||
            moveAction.row_to < 0 || moveAction.row_to >= 100)
        {
            throw "矩阵元素坐标越界。";
        }

        int d_row = abs(moveAction.row_to - moveAction.row_from);
        int d_col = abs(moveAction.col_to - moveAction.col_from);
        if(  d_row > 1  || d_col > 1 ||
             (d_row == 1 && d_col == 1))
        {
            throw "矩阵元素只能上下左右置换。";
        }
    }
    
    /**
     * 移动矩阵元素
     * @param matrix
     * @param moveActions
     * @return
     * @throws Exception
     */
    void MoveMatrix(int matrix[][100], list<MoveAction> moveActions){
        if( moveActions.size() > 0)
        {
            for(list<MoveAction>::iterator i = moveActions.begin() ; i !=  moveActions.end() ; i++ )
            {
            
                MoveAction moveAction = (*i);

                CheckValid(moveAction);

                int from = matrix[moveAction.row_from][moveAction.col_from];
                int to = matrix[moveAction.row_to][moveAction.col_to];

                matrix[moveAction.row_from][moveAction.col_from] = to;
                matrix[moveAction.row_to][moveAction.col_to] = from;
            }
        }        
    }
    
    /**
     * 判断该矩阵所有1是否形成一条直线
     * @param matrix
     * @return
     */
    bool IsLineMatrix( int matrix[][100] )
    {
        bool ret;

        list<ElementLocation> elementLocations;

        //先找出所有1的元素
        for( int i=0; i < 100; i++ )
        {
            for( int j=0; j < 100; j++ )
            {
                if( matrix[i][j] == 1  )
                {
                    elementLocations.push_back(ElementLocation(i,j));
                }
            }
        }

        //直线类型：横线
        bool isTransverseLine = true;
        ElementLocation preLocation(-1,-1);
        ElementLocation curLocation(-1,-1) ;
        for( list<ElementLocation>::iterator i = elementLocations.begin(); i != elementLocations.end(); i++ )
        {
            curLocation = (*i);

            if( preLocation.col == -1 &&  preLocation.row == -1 )
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
        bool isVerticalLine = true;
        preLocation.row = -1;
        preLocation.col = -1;
        
        for(list<ElementLocation>::iterator i = elementLocations.begin(); i != elementLocations.end(); i++)
        {
            curLocation = (*i);

            if( preLocation.row == -1 && preLocation.col == -1 )
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
        bool isUpDiagonalLine = true;
        preLocation.row = -1;
        preLocation.col = -1;
        
        for( list<ElementLocation>::iterator i = elementLocations.begin(); i != elementLocations.end(); i++)
        {
            curLocation = (*i);

            if( preLocation.row == -1 && preLocation.col == -1 )
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
        bool isDownDiagonalLine = true;
        preLocation.row = -1;
        preLocation.col = -1;
        
        for( list<ElementLocation>::iterator i = elementLocations.begin(); i != elementLocations.end(); i++)
        {
            curLocation = (*i);

            if( preLocation.row == -1 && preLocation.col == -1 )
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
   


	void life_the_universe_and_everthing()
	{	
		
		int num_of_matrixs = 1000;
		//耗费步数
        int step_count = 0;
        double time_total = 0.0;

        for( int count_matrix = 1; count_matrix <= num_of_matrixs; count_matrix++ )
        {
            //读取矩阵
            string filename;
            
            stringstream ss;
            
            ss<<"../matrices/matrix"<<count_matrix<<".txt";
            ss>>filename;
						  
		
            int matrix[100][100];
			readMatrixFile(matrix,filename);

            int matrix_copy[100][100];

            //拷贝二维数组
            copy2DArray(matrix, matrix_copy);

            clock_t startTime = clock();
            list<MoveAction> moveActions = DrawLine(matrix_copy);
            clock_t endTime = clock();
            
            time_total += (double)(endTime - startTime) / CLOCKS_PER_SEC * 1000;        

           

            try {

                MoveMatrix(matrix, moveActions);
                
                if (IsLineMatrix(matrix) == false) {
                    throw "置换后的矩阵，所有1元素没有形成一条直线。";
                }

                step_count += (moveActions.size());

                std::cout<<"测试矩阵"<<filename<<"成功"<<std::endl;
                std::cout<<"耗费步数:"<<step_count<<std::endl;

            } catch (const char* msg) {
                std::cout<<"测试矩阵"<<filename<<"失败"<<std::endl;
                std::cout<< msg << std::endl;
            }
        }

        std::cout<<"总步数:"<< step_count << std::endl;
        std::cout<<"总耗时:"<< (int)time_total <<  "ms" << std::endl;
	}


int main()
{
	life_the_universe_and_everthing();
	//system("pause");
}
