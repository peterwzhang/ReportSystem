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
    String reportFileName;
    ReportThreadHandler(int intId, String fileName){
        id = intId;
        reportFileName =  fileName;
    }
    public void run(){
        System.out.println("Created thread ID: " + id + " for report: " + reportFileName); // change to stderr printing later
        try {
            //parse specification file then write report request
            File reportSpecFile = new File(reportFileName);
            Scanner reportSpec = new Scanner(reportSpecFile);

            //parse here

            //MessageJNI.writeReportRequest(id, reportCount, searchString);
            reportSpec.close();
        } catch(FileNotFoundException ex) {
			    System.out.println("FileNotFoundException triggered:"+ex.getMessage());
        };
    }
}