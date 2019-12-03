
public class ElementLocation{
    public int row;
    public int col;

    public ElementLocation(int row,int col)
    {
        this.row = row;
        this.col = col;
    }

    public ElementLocation clone(){
        ElementLocation clone = new ElementLocation(this.row,this.col);
        return clone;
    }
}
