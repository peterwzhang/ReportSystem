package edu.cs300;

import java.util.Vector;

public class Report{
    String reportTitle;
    String searchString;
    String outputFName;
    Vector<ColumnField> colFields;

    Report(String lOne, String lTwo, String lThree){
        reportTitle = lOne;
        searchString = lTwo;
        outputFName = lThree;
        colFields = new Vector<ColumnField>();
        DebugLog.log("created " + lOne + "\n" + lTwo + '\n' + lThree);
    }

    void addLine(String line){
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
            // yeah so I chose not to handle this because I really hope you dont break my program
            // Tom West Law - "Don't always do it perfectly"
            // Lauer's Law - "Less code is better code"
            // thats why I wrote no code :)
        }
        DebugLog.log(cF.toString());
        colFields.add(cF);
    }
}