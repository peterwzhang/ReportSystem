package edu.cs300;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Vector;

public class Report{
    private String reportTitle;
    private String searchString;
    private String outputFName;
    private Vector<ColumnField> colFields;
    private Vector<String> lines;

    public Report(String lOne, String lTwo, String lThree){
        reportTitle = lOne;
        searchString = lTwo;
        outputFName = lThree;
        colFields = new Vector<ColumnField>();
        lines = new Vector<String>();
        //DebugLog.log("created " + lOne + "\n" + lTwo + '\n' + lThree);
    }

    public void setReportTitle(String rt){
        reportTitle = rt;
    }

    public String getReportTitle(){
        return reportTitle;
    }

    public void setSearchString(String ss){
        searchString = ss;
    }

    public String getSearchString(){
        return searchString;
    }

    public void setFileName(String fn){
        outputFName = fn;
    }

    public String getFileName(){
        return outputFName;
    }

    // this method assumes report spec files are corectly formatted as per the project pdf
    public void addCol(String line){
        int dashIndex = line.indexOf('-');
        int commaIndex = line.indexOf(',');
        int startIndex, endIndex;
        String name = line.substring(commaIndex + 1);
        ColumnField cF = new ColumnField();
        try {
            startIndex = Integer.parseInt(line.substring(0, dashIndex));
            endIndex = Integer.parseInt(line.substring(dashIndex + 1, commaIndex));
            cF = new ColumnField(startIndex, endIndex, name);
        }
        catch (NumberFormatException e) {
            System.exit(1);
        }
        //DebugLog.log(cF.toString());
        colFields.add(cF);
    }

    public void addLine(String line){
        String newLine = new String();
        for (ColumnField col : colFields){
            String newCol = line.substring(col.getStartIndex(), col.getEndIndex());
            newCol = newCol+ '\t';
            newLine += newCol;
        }
        lines.add(newLine + '\n');
    }

    public void printReport(){
        try {
        FileWriter fw = new FileWriter(outputFName);
        fw.write(reportTitle + '\n');

        // print column header
        String colString = new String();
        for (ColumnField col : colFields){
            colString += col.getName();
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
            System.exit(1);
        }
    }
}