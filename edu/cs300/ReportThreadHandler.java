package edu.cs300;

import java.io.File;
import java.io.FileNotFoundException;
import java.lang.Runnable;
import java.util.Scanner;

public class ReportThreadHandler implements Runnable{
    private int id;
    private int numReports;
    private String reportFileName;
    public ReportThreadHandler(int intId, int reportCount, String fileName){
        id = intId;
        numReports = reportCount;
        reportFileName =  fileName;
    }
    public void setId(int newId){
        id = newId;
    }
    public void setNumReports(int newNum){
        numReports = newNum;
    }
    public void setRepFileName(String newName){
        reportFileName = newName;
    }
    public int getId(){
        return id;
    }
    public int getNumReports(){
        return numReports;
    }
    public String getRepFileName(){
        return reportFileName;
    }
    public void run(){
        System.out.println("Created thread ID: " + id + " for report: " + reportFileName); // change to stderr printing later
        try {
            //parse specification file then write report request
            File reportSpecFile = new File(reportFileName);
            Scanner reportSpec = new Scanner(reportSpecFile);
            //parse here
            Report report = new Report(reportSpec.nextLine(), reportSpec.nextLine(), reportSpec.nextLine());
            while(reportSpec.hasNextLine()){
                //TODO: see if we can improve this
                String nextLine = reportSpec.nextLine();
                if (!reportSpec.hasNextLine()){ // for last line
                    break;
                }
                report.addCol(nextLine);
            }
            reportSpec.close();

            MessageJNI.writeReportRequest(id, numReports, report.getSearchString());
            while (true){
                String readString = MessageJNI.readReportRecord(id);
                //DebugLog.log("read string: " +readString);
                if (readString.isEmpty()){ break; }
                report.addLine(readString);
            }
            report.printReport();
        } catch(FileNotFoundException ex) {
			    System.exit(1); // exit gracefully if files DNE
        };
    }
}