package edu.cs300;

import java.io.Console;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Enumeration;
import java.util.Scanner;
import java.util.Vector;


public class ReportingSystem {


	public ReportingSystem() {
	    DebugLog.log("Starting Reporting System");
	}

	public int loadReportJobs() {
		int reportCounter = 0;
		try {

			    File file = new File ("report_list.txt");

			    Scanner reportList = new Scanner(file);
                //assuming report list will always be valid
                reportCounter = reportList.nextInt();
                reportList.nextLine();
                //id =  vector index + 1
                Vector<String> reportNames = new Vector<String>(reportCounter);
                while(reportList.hasNext()){
                    reportNames.add(reportList.nextLine());
                }
                DebugLog.log(reportNames.toString()); // added this debug
 		        //load specs and create threads for each report
				DebugLog.log("Load specs and create threads for each report\nStart thread to request, process and print reports");
                for (int index = 0; index < reportCounter; ++index){
                    ReportThreadHandler RTH = new ReportThreadHandler(index + 1, reportCounter, reportNames.elementAt(index));
                    Thread rthThread = new Thread(RTH);
                    rthThread.start();
                }

			    reportList.close();
		} catch (FileNotFoundException ex) {
			    System.out.println("FileNotFoundException triggered:"+ex.getMessage());
		}
		return reportCounter;

	}

	public static void main(String[] args) throws FileNotFoundException {


		   ReportingSystem reportSystem = new ReportingSystem();
		   int reportCount = reportSystem.loadReportJobs();


	}

}
