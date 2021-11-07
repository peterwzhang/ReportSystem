package edu.cs300;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Vector;

public class Report{
    String reportTitle;
    String searchString;
    String outputFName;
    Vector<ColumnField> colFields;
    Vector<String> lines;

    Report(String lOne, String lTwo, String lThree){
        reportTitle = lOne;
        searchString = lTwo;
        outputFName = lThree;
        colFields = new Vector<ColumnField>();
        lines = new Vector<String>();
        DebugLog.log("created " + lOne + "\n" + lTwo + '\n' + lThree);
    }

    void addCol(String line){
        int dashIndex = line.indexOf('-');
        int commaIndex = line.indexOf(',');
        int startIndex, endIndex;
        String name = line.substring(commaIndex + 1);
        ColumnField cF = new ColumnField();
        try {
            // this assumes the report files will always be valid
            startIndex = Integer.parseInt(line.substring(0, dashIndex));
            endIndex = Integer.parseInt(line.substring(dashIndex + 1, commaIndex));
            cF = new ColumnField(startIndex, endIndex, name);
        }
        catch (NumberFormatException e) {
            //TODO: handle this
        }
        DebugLog.log(cF.toString());
        colFields.add(cF);
    }

    void addLine(String line){
        String newLine = new String();
        for (ColumnField col : colFields){
            String newCol = line.substring(col.startIndex - 1, col.endIndex);
            newCol = newCol+ '\t';
            newLine += newCol;
        }
        lines.add(newLine + '\n');
    }

    void printReport(){
        try {
        FileWriter fw = new FileWriter(outputFName);
        fw.write(reportTitle + '\n');

        // print column header
        String colString = new String();
        for (ColumnField col : colFields){
            colString += col.name;
            colString += '\t';
        }
        colString += '\n';
        fw.write(colString);

        // print all lines
        for (String line : lines){
            fw.write(line);
        }

        fw.close();
        //DebugLog.log("printed " + outputFName);
        }
        catch (IOException e){
            //TODO: implement
        }
    }
}