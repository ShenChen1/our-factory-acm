

public class MoveAction {

    public int row_from;
    public int col_from;
    public int row_to;
    public int col_to;

    public MoveAction(int row_from,int col_from,int row_to,int col_to)
    {
        this.row_from = row_from;
        this.col_from= col_from;
        this.row_to = row_to;
        this.col_to =col_to;
    }

    public MoveAction clone()
    {
        return new MoveAction(this.row_from,this.col_from,this.row_to,this.col_to);
    }
}
