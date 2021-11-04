package edu.cs300;

import java.io.File;
import java.io.FileNotFoundException;
import java.lang.Runnable;
import java.util.Scanner;

/*
I decided to use implements runnable instead of extends thread
since I think this project is very interesting. In the case I
want ReportThreadHandler to inherit another class I would like
that option to be open. :)
 */

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
                report.addLine(nextLine);
            }
            
            MessageJNI.writeReportRequest(id, numReports, report.searchString);
            reportSpec.close();
        } catch(FileNotFoundException ex) {
			    System.out.println("FileNotFoundException triggered:"+ex.getMessage());
        };
    }
}