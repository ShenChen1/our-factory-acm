#ifndef HIKER_INCLUDED
#define HIKER_INCLUDED

#include <string>
#include <list>
#include <iostream>
using namespace std;


class ElementLocation{
    public: int row;
    public: int col;

    public: ElementLocation(int row,int col)
    {
        this->row = row;
        this->col = col;
    }
    
    public: ElementLocation(const ElementLocation& s)
    {
    	row = s.row;
    	col = s.col;
	}
    
};

class MoveAction {

    public: int row_from;
    public: int col_from;
    public: int row_to;
    public: int col_to;

    public: MoveAction(int row_from,int col_from,int row_to,int col_to)
    {
        this->row_from = row_from;
        this->col_from= col_from;
        this->row_to = row_to;
        this->col_to =col_to;
    }    
    
    public: MoveAction(const MoveAction& s)
    {
    	row_from = s.row_from;
    	col_from = s.col_from;
    	col_to = s.col_to;
    	row_to = s.row_to;
	}
	
	
};

std::list<MoveAction> DrawLine(int maxtrix[][100]);

#endif
