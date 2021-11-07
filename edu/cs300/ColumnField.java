package edu.cs300;


public class ColumnField {
    private int startIndex;
    private int endIndex;
    String name;

    public ColumnField(){
        startIndex = 0;
        endIndex = 0;
        name = "";
    }

    public ColumnField(int sIndex, int eIndex, String cName){
        startIndex = sIndex;
        endIndex = eIndex;
        name = cName;
    }

    public void setStartIndex(int index){
        startIndex = index;
    }

    public void setEndIndex(int index){
        endIndex = index;
    }

    public int getStartIndex(){
        if (startIndex <= 0) return 0;
        return startIndex - 1; //indices on report spec start at 1 but string index start at 0
    }

    public int getEndIndex(){
        if (endIndex <= 0) return 0;
        return endIndex; // substring uses exclusive for the end index so we don't need to subtract 1
    }
    @Override
    public String toString(){
        return String.format("S: %d, E: %d, Name: %s\n", startIndex, endIndex, name);
    }
}
