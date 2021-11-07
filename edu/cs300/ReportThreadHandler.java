package edu.cs300;

import java.io.File;
import java.io.FileNotFoundException;
import java.lang.Runnable;
import java.util.Scanner;

public class ReportThreadHandler implements Runnable{
    int id;
    int numReports;
    String reportFileName;
    ReportThreadHandler(int intId, int reportCount, String fileName){
        id = intId;
        numReports = reportCount;
        reportFileName =  fileName;
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

            MessageJNI.writeReportRequest(id, numReports, report.searchString);
            while (true){
                String readString = MessageJNI.readReportRecord(id);
                //DebugLog.log("read string: " +readString);
                if (readString.isEmpty()){ break; }
                report.addLine(readString);
            }
            report.printReport();
        } catch(FileNotFoundException ex) {
			    System.out.println("FileNotFoundException triggered:"+ex.getMessage());
        };
    }
}