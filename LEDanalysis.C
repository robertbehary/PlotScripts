//*********************************************************************
//
//                         Duquesne University
//                            Robert Behary
//
// A program that makes "Run" objects that can be used to
// plot data from LED data text files in the format ftCalLed_-*****_
// when given a directory where all of these text files are located.
//
// Specifically this script makes three plots of:
// 1.) component# vs amplitude mean
// 2.) component# vs ratio of run/earliest run in the directory
// 3.) radial distance vs ratio of runs
//
// Ratios are computed by searching through the directory for the
// lowest run number among the files. For desired result, set to a
// directory that has the runs that you want to compare. High number
// of files may over clutter graphs.
//
// To run this program, you only need to set the path near the bottom
// of the file. The lines are surrounded by asteriscs "****". Two variables
// are directory and path. These will be the same, but path has a '/' at the
// end of the copied path while directory omits the final '/'.
//
//**********************************************************************

// Notes for future use:
// The run number is limited to 5 digits, so you may have to change the string command in the Run constructor.
// Making other plots should be easy just by copying the template of the graphs made here.
// Large numbers of runs makes the legend titles very small may have to work on fixing that for many comparisons.
// This ill also not like it if there are other files in the directory that don't fit the pattern.

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <stdlib.h>
using namespace std;

class Run {

  // convenience variables for tracking
  string filename;
  string runNumber;
  int runNumberInt;

  // variables of the data file
  int num = 332; // 332 entries in the tables
  int numMod = 39; // 39 unique values for radial distances
  Double_t sector[332], 
           layer[332],
           component[332],
           pedistal[332],
           noise[332],
           charge[332],
           chargeSigma[332],
           ampMean[332],
           ampSigma[332]; // variables from the text file all here for future plots if desirable

  // Variables for position of crystals
  int x[332],
      y[332];
  Double_t r[332],
           rOrdered[332],
           rOrderedCut[39]; // 39 unique distances from the crystals
  std::vector<double> rSorted; // make unique elements from sorted array

  // Variables for plotting purposes
  Double_t ratio[332];
  std::vector<double> orderedRatios;
  Double_t orderedRatioArray[332];
  std::vector<double> average;
  std::vector<double> error;
  Double_t averageArray[39],
           errorArray[39];

public:

  // constructor for Run object
  Run(const string filename) {
    
    // run number in both string and integer format based of file name convention
    int pos = filename.find("ftCalLed_-");
    runNumber = filename.substr(pos+10,5);
    runNumberInt = stoi(runNumber);

    // setting up the file to read                                                                   
    ifstream in;
    in.open(filename);

    if (in.is_open()) {
      // reading each column into array with its data
      for (int i = 0; i < num; i++) {
	in >> sector[i] >>
	  layer[i] >>
	  component[i] >>
	  pedistal[i] >>
	  noise[i] >>
	  charge[i] >>
	  chargeSigma[i] >>
	  ampMean[i] >>
	  ampSigma[i];
      }
      in.close();
    }
    else {
      cout << "error reading file" << endl;
      return;
    }
    
    for (int i = 0; i < num; i++) {
      // way to get x and y coordinates from components
      // formula from Raffaella (slides of FT-Cal somewhere?)
      y[i] = (int)(component[i]/22) + 1;
      x[i] = (int)(component[i] + 1 - (y[i]-1)*22);

      // shifting the x and y components to match crystal position coordinates
      if (y[i] <= 11)
	y[i] = y[i] - 12;
      else y[i] = y[i] - 11;
      if (x[i] <= 11)
        x[i] = x[i] - 12;
      else x[i] = x[i] - 11;
      
      // getting the radial distance r for each component
      r[i] = sqrt(x[i]*x[i] + y[i]*y[i]);

    }
    
    // Sorting array for use
    int n = sizeof(r)/sizeof(r[0]); // gets the size of the array
    for (int i = 0; i < n ; i++) {  // puts the contents into a new array
      rOrdered[i] = r[i];
    }
    sort(rOrdered,rOrdered+n); // sorts new array

    // rSorted is an array that is both in order in terms of radial distance and of the 
    // correct length so that there are no duplicates
    std::vector<double> rSorted (rOrdered, rOrdered+332); // make unique elements from sorted array
    std::vector<double>::iterator it;
    it = std::unique(rSorted.begin(), rSorted.end());
    rSorted.resize(std::distance(rSorted.begin(),it));

    // makes vector into an array we can use easily
    for (int i = 0; i < rSorted.size(); i++) {
      rOrderedCut[i] = rSorted[i];
    }
    
  }
  
  // default constructor (needed for array of "Run" later on)
  Run() {}
  
  // for testing to see if the graph is correct just to make sure you have the right file
  // unused, but good to keep around to check if curious
  void display() {
    for(int i = 0; i < 10 ; i++) {
      cout << ampMean[i] << " ";
    }
    cout << " test " << runNumber << endl;
  }
  
  // method that returns a TGraph of the Component Number vs. the Mean Amplitude
  TGraphErrors meanAmpGraph(int total, int color) {

    // astetic choice to make colors in the same range, but markers different
    // useful if there are a lot of runs
    int marker;
    if (color < 10) {
      marker = 20;
    }
    else {
      color = color - 9;
      marker = 21;
    }

    //run being processed by name
    char runNumberGraph[runNumber.size()+1]; // making a string with run number from file name
    copy(runNumber.begin(), runNumber.end(), runNumberGraph);
    runNumberGraph[runNumber.size()] = '\0';

    // making the graph and setting the color and marker style
    auto graph = new TGraphErrors(num, component, ampMean, nullptr,nullptr);
    graph -> SetMarkerStyle(marker);
    graph -> SetMarkerColor(color);
    graph -> SetDrawOption("AP");
    graph -> SetTitle(runNumberGraph);

    return *graph;
  }
  
  // Finds the lowest run in the directory
  Run* findLowestRun(Run* lowest) {
    if (lowest->runNumberInt > this->runNumberInt) {
      return this;
    }
    else
      return lowest;
  }
  
  // conditional used to put the runs in order of ascending number
  bool lowestTest(Run* current) {
    return (this -> runNumberInt) > (current -> runNumberInt);
  }
  
  // Sets the ratio for each run
  void setRatio(Run* lowest) {
    for (int i = 0; i<num; i++) {
      this->ratio[i] = this->ampMean[i] / lowest->ampMean[i];
    } 
  }

  // method that returns a TGraph of the Component Number vs. the Mean Amplitude Ratio
  TGraphErrors meanAmpRatioGraph(Run* lowest, int total, int color) {

    // astetic choice to make colors in the same range, but markers different
    // useful if there are a lot of runs                           
    int marker;
    if (color < 10) {
      marker = 20;
    }
    else {
      color = color - 9;
      marker = 21;
    }

    // block to make an informative legend label
    char runNumberGraph[runNumber.size()+1]; // making a string with run number from file name
    copy(runNumber.begin(), runNumber.end(), runNumberGraph);
    runNumberGraph[runNumber.size()] = '\0';
    char runNumberGraphLowest[runNumber.size()+1]; // string of the lowest run number
    copy(lowest->runNumber.begin(), lowest->runNumber.end(), runNumberGraphLowest);
    runNumberGraphLowest[runNumber.size()] = '\0';
    string runNumberGraphString = string(runNumberGraph); // concatinating the strings
    string runNumberGraphLowestString = string(runNumberGraphLowest);
    string graphTitleString = runNumberGraphString + "/" + runNumberGraphLowestString;
    char graphTitle[graphTitleString.size()+1];
    strcpy(graphTitle,graphTitleString.c_str()); // final name

    // making the graph and setting the color and marker style
    auto graph = new TGraphErrors(num, component, ratio, nullptr,nullptr);
    graph -> SetMarkerStyle(marker);
    graph -> SetMarkerColor(color);
    graph -> SetDrawOption("AP");
    graph -> SetTitle(graphTitle);

    return *graph;
  }

  // Setting average and standard deviation based on ratios for radial distance plots
  void setParameters() {

    // loop that puts ratios in order based on rSorted
    std::vector<double>::iterator it;
    for (int i = 0; i < 39; i++) { // 39 unique distances from the crystals
      for (int j = 0; j < num; j++) {
	if(rOrderedCut[i] == r[j]) {
	  orderedRatios.push_back(ratio[j]);
	}
      }
    }

    // making vector into an array for ease of use
    for (int i = 0; i < orderedRatios.size(); i++) {
      orderedRatioArray[i] = orderedRatios[i];
    }

    // loops to find the average and standard deviation per spot on the radial distance
    int count = 0, start = 0, last = 0, trk = 0; // indicies
    double avg = 0, std = 0, sum = 0; // variables
    for (trk; trk < 39; trk++) {
      sum = 0;
      count = 0;
      start = last;
      for (last; last < num; last++) {
	if (rOrderedCut[trk] == rOrdered[last]) { // go until the values do not match
	  sum = sum + orderedRatioArray[last];
	  count++;
	}
	else { // make average and standard deviation based on indicies above
	  avg = sum/count;
	  average.push_back(avg);
	  std = getError(avg,count,start,last - 1);
	  error.push_back(std);
	  break;
	}
	if (last == 331) { // special case if you are at the end do the above with the rest of the array
          avg = sum/count;
          average.push_back(avg);
          std = getError(avg,count,start,last - 1);
          error.push_back(std);
          break;
        }
      }
    }
    
    // put these values into an array to plot later
    // same size, so we can use one loop
    for (int i = 0; i < average.size(); i++) {
      averageArray[i] = average[i];
      errorArray[i] = error[i];
    }
    
  }

  // standard deviation function taken from stackoverflow
  double getError(double avg, int count, int start, int last) {
    double var = 0;
    double sd = 0;
    for(start; start < last + 1; start++) {
      var = var + ((orderedRatioArray[start] - avg) * (orderedRatioArray[start] - avg));
    }
    var /= count;
    sd = sqrt(var);
    return sd;
  }

  // method that returns a TGraph of the Radial Distance vs. the Mean Amplitude Ratio average with standard
  // deviation on the error bars
  TGraphErrors radialDistanceGraph(Run* lowest) {

    // block to make informative graph title name
    char runNumberGraph[runNumber.size()+1]; // making a string with run number from file name
    copy(runNumber.begin(), runNumber.end(), runNumberGraph);
    runNumberGraph[runNumber.size()] = '\0';
    char runNumberGraphLowest[runNumber.size()+1]; // string of lowest run number
    copy(lowest->runNumber.begin(), lowest->runNumber.end(), runNumberGraphLowest);
    runNumberGraphLowest[runNumber.size()] = '\0';
    string runNumberGraphString = string(runNumberGraph); // concatinating the strings
    string runNumberGraphLowestString = string(runNumberGraphLowest);
    string graphTitleString = "Run " + runNumberGraphString + "/" + runNumberGraphLowestString + " Radial Distance";
    char graphTitle[graphTitleString.size()+1]; // final name
    strcpy(graphTitle,graphTitleString.c_str());
    
    // making the graph and setting the color and marker style
    auto graph = new TGraphErrors(numMod, rOrderedCut, averageArray, nullptr,errorArray);
    graph -> SetMarkerStyle(21);
    graph -> SetMarkerColor(4);
    graph -> SetDrawOption("AP");
    graph -> SetTitle(graphTitle);
    graph->SetMinimum(0.9); // setting range for y-axis becuase ratios may be off
    graph->SetMaximum(1.1); // adjust these as needed for averages
    graph->GetXaxis()->SetTitle("Radial Distance [arb. units]");
    graph->GetYaxis()->SetTitle("Amplitude Mean Ratio Average");

    return *graph;
  }

};

// main method of the file where commands are given and directory is changed
void LEDanalysis() {
  
  //*************************************************************
  // change these two line to the path where files are kept
  // the string for path will be the directory with the "/" as the final character
  string path = "/Users/robertbehary/Genoa/clas12calibration-ft/OriginalLEDData/";
  char directory[] = "/Users/robertbehary/Genoa/clas12calibration-ft/OriginalLEDData";
  // only need to change these two lines for this to work
  //************************************************************

  // getting all of the files from the directory into an array
  char extension[] = ".txt";
  int i = 0;
  string testFiles[30];
  TSystemDirectory dir(directory,directory);
  TList *files = dir.GetListOfFiles();
  if (files) {
    TSystemFile *file;
    TString fname;
    TIter next(files);
    while ((file = (TSystemFile*)next())) {
      fname = file -> GetName();
      if (!file -> IsDirectory() && fname.EndsWith(extension)) {
	testFiles[i] = fname.Data();
	i++;
      }
    }
  }

  // makes an array of all the runs with the file name
  // set the initial size to 30, but change that as you may need more files
  Run *collection[30];
  string currentRun;
  for (int j = 0; j < i; j++) {
    currentRun = path + testFiles[j];
    collection[j] = new Run(currentRun);
  }
  cout << "All Files in Directory Processed. " << endl;

  // ordering the list to present it in chronological order in the plots
  // just in case the program reads the files in incorrectly
  // buble sort from c++ website
  Run* current = collection[0];
  int spot = 0, slop = 0, flag = 1;
  for (spot = 1; (spot < i) && flag; spot++) {
    flag = 0;
    for (slop = 0; slop < i-1; slop++) {
      if (collection[slop]->lowestTest(collection[slop+1])) {
        current = collection[slop];
        collection[slop] = collection[slop+1];
        collection[slop+1] = current;
        flag =1;
      }
    }
  }
  cout << "Runs are in ascending order." << endl;

  // loop to make mean amp plot
  auto meanAmpPlot = new TCanvas("meanAmpPlot","meanAmpPlot",600,400);
  TGraph *g[i];                                                                                            
  TMultiGraph *mg = new TMultiGraph();
  for (int j = 0; j < i; j++) {
    g[j] = (TGraphErrors*)(collection[j] -> meanAmpGraph(i,j+1)).Clone();
    mg -> Add(g[j]);
  }
  mg->SetTitle("Amplitude Mean vs. Component Number;Component Number;Amplitude Mean [mV]");                    
  mg->Draw("AP");                                                                                              
  meanAmpPlot -> BuildLegend();
  cout << "Mean Amp Plot Finished." << endl;
  
  // finding the lowest run number in the set just in case files not in order
  Run* lowest = collection[0];
  for (int j = 0; j<i; j++) {
    lowest = (Run*)(collection[j]->findLowestRun(lowest));
  }
  
  // Loop to set the ratio of each run
  for (int j = 0; j<i; j++) {
    collection[j]->setRatio(lowest);
  }
  
  // loop to make mean amp ratio plot
  auto meanAmpRatioPlot = new TCanvas("meanAmpRatioPlot","meanAmpRatioPlot",600,400);
  TGraph *g1[i];
  TMultiGraph *mg1 = new TMultiGraph();
  for (int j = 0; j < i; j++) {
    g1[j] = (TGraphErrors*)(collection[j] -> meanAmpRatioGraph(lowest,i,j+1)).Clone();
    mg1 -> Add(g1[j]);
  }
  mg1->SetTitle("Amplitude Mean Ratio vs. Component Number;Component Number;Amplitude Mean Ratio");
  mg1->Draw("AP");
  mg1->SetMinimum(0.8); // setting range for y-axis becuase ratios may be off
  mg1->SetMaximum(1.1);
  meanAmpRatioPlot -> BuildLegend();
  cout << "Mean Amp Ratio Plot Finished." << endl;

  // loop to make radial distance plots
  // check to get the rows right in the graph
  int rows = 0;
  if (i%3 >= 1) {
    rows = (i/3) + 1;
  }
  else rows = i/3;

  auto radialDistancePlot = new TCanvas("radialDistancePlot","radialDistancePlot",900,700);
  TGraph *g2[i];
  radialDistancePlot -> Divide(3,rows);
  for (int j = 0; j < i; j++) {
    collection[j] -> setParameters();
    g2[j] = (TGraphErrors*)(collection[j] -> radialDistanceGraph(lowest)).Clone();
    radialDistancePlot -> cd(j+1);
    g2[j] -> Draw("AP");
  }
  cout << "Radial Distance Plot Finished." << endl;

}
