package edu.cs300;


public class ColumnField {
    int startIndex;
    int endIndex;
    String name;

    ColumnField(){
        startIndex = 0;
        endIndex = 0;
        name = "";
    }

    ColumnField(int sIndex, int eIndex, String cName){
        startIndex = sIndex;
        endIndex = eIndex;
        name = cName;
    }

    @Override
    public String toString(){
        return String.format("S: %d, E: %d, Name: %s\n", startIndex, endIndex, name);
    }
}
