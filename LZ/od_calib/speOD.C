/**
 * # Macro to determine spe area of OD PMTs (Theresa Fruth, 20 April 2018)
 * Based on example macro from MDC2 kick-off
 * This macro reads all the files listed in the file `RQfile.list` and generate an output rootfile `speOD.root`.
 * ## Usage: 
 * Creation of the Rqlib, need to do only once (or as RQEvent format changes). 
 * ```bash
 * root -b -q configure.C
 * ```
 * Run the analysis macro:
 * ```bash
 *root -b -q load.C macro.C+ 
 * ```
 *
 *
 * The output file contains all the histograms ceated during the processing.
 * When you use the comman ```root -b -q load.C macro.C+```, root will automaticaly execute the function `macro`.
 *
 *---
 *# Description of the macro
 * 
 * In the following section, each part of the macro will be described. The major blocks are: 
 * 1. [Headers] (#Headers)
 * 2. [Event structure] (#event_struct)
 * 3. [Cut definition] (#cuts)
 * 4. [Main executable] (#main)
 * 5. [Load chain] (#load)
 * 6. [Event loop] (#loop)
 *
 */


/**
 * ## Headers <a name="Headers"></a>
 * Include all the headers
 *
 */
// Standard C++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// ROOT Headers
#include "TROOT.h"
#include "TSystem.h"
#include "TStopwatch.h"
#include "TFile.h"
#include "TString.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TMath.h"
#include "TTimeStamp.h"
#include "TGraphErrors.h"

// Event structure header
#include "rqlib/LZapRQ__RQEvent.h"

// STD namespace
using namespace std;

// Event structure namespace
using namespace LZapRQ;

// Functions prototype
void load_chain(TString txtFileList, TChain* chain);
void eventLoop(TChain* events);


/**
 * ## Outputs
 * - `outname`: Name of the output file
 * - `outfile`: Pointer to the output TFile
 */

TString outname = "speOD.root";
TFile* outfile;


/**
 * ## Lite event structure <a name="event_struct"></a>
 *
 * Description:
 *   Keep in this event structure only the required RQs for this analysis. 
 *
 * ### Variables: 
 * - `nHGPulses`: Counter of the number of High Gain pulses
 * 
 * ### Methods:
 * - `Set(RQEvent* evt)`: Extract the required RQs from RQEvent event class
 *
 */

struct MyEvent
{
  //------------------------------------------------
  // simple struct to hold variables derived from RQs
  //------------------------------------------------
  void SetVariables(RQEvent* evt){

    //------------------------------------------------
    // RQs from HGPulses and LGPulses
    //------------------------------------------------
    nHGPulses = evt->ODHGPulses.size();

    // Reset the RQs
    HGPeakAmp.resize(nHGPulses);
    HGPeakTime.resize(nHGPulses);
    HGArea.resize(nHGPulses);
    HGrmsWidth.resize(nHGPulses);

    hg_ch_area_phd.resize(nHGPulses);
    hg_ch_peak_phd.resize(nHGPulses);
    hg_ch_ID.resize(nHGPulses);
    hg_ch_PeakTime.resize(nHGPulses);
    hg_ch_Saturated.resize(nHGPulses);

    nLGPulses = evt->ODLGPulses.size();

    // Reset the RQs
    LGPeakAmp.resize(nLGPulses);
    LGPeakTime.resize(nLGPulses);
    LGArea.resize(nLGPulses);
    LGrmsWidth.resize(nLGPulses);

    lg_ch_area_phd.resize(nLGPulses);
    lg_ch_peak_phd.resize(nLGPulses);
    lg_ch_ID.resize(nLGPulses);
    lg_ch_PeakTime.resize(nLGPulses);
    lg_ch_Saturated.resize(nLGPulses);
    
    
    //------------------------------------------------
    // Loop over all the high gain pulses
    //------------------------------------------------
    for(int i=0; i<nHGPulses; ++i){
      //Fill pulse variables
      HGPeakAmp[i]  = evt->ODHGPulses[i].peakAmp;
      HGPeakTime[i] = evt->ODHGPulses[i].peakTime;
      HGArea[i]     = evt->ODHGPulses[i].area_phd;
      HGrmsWidth[i] = evt->ODHGPulses[i].rmsWidth;
      //Fill channel pulse variables 
      int nChPulses = evt->ODHGPulses[i].chArea.size();
      hg_ch_area_phd[i].resize(nChPulses);
      hg_ch_peak_phd[i].resize(nChPulses);
      hg_ch_ID[i].resize(nChPulses);
      hg_ch_PeakTime[i].resize(nChPulses);
      hg_ch_Saturated[i].resize(nChPulses);
      for(int n = 0; n < nChPulses; ++n) {
         hg_ch_area_phd[i][n] = evt->ODHGPulses[i].chArea[n];
         hg_ch_peak_phd[i][n] = evt->ODHGPulses[i].chPeakAmp[n];
         hg_ch_ID[i][n]       = evt->ODHGPulses[i].chID[n];
         hg_ch_PeakTime[i][n] = evt->ODHGPulses[i].chPeakTime[n];
         hg_ch_Saturated[i][n] = evt->ODHGPulses[i].chSaturated[n];
      }
    }    

    //------------------------------------------------
    // Loop over all the low gain pulses
    //------------------------------------------------
    for(int i=0; i<nLGPulses; ++i){
      //Fill pulse variables
      LGPeakAmp[i]  = evt->ODLGPulses[i].peakAmp;
      LGPeakTime[i] = evt->ODLGPulses[i].peakTime;
      LGArea[i]     = evt->ODLGPulses[i].area_phd;
      LGrmsWidth[i] = evt->ODLGPulses[i].rmsWidth;
      //Fill channel pulse variables 
      int nChPulses = evt->ODLGPulses[i].chArea.size();
      lg_ch_area_phd[i].resize(nChPulses);
      lg_ch_peak_phd[i].resize(nChPulses);
      lg_ch_ID[i].resize(nChPulses);
      lg_ch_PeakTime[i].resize(nChPulses);
      lg_ch_Saturated[i].resize(nChPulses);
      for(int n = 0; n < nChPulses; ++n) {
         lg_ch_area_phd[i][n] = evt->ODLGPulses[i].chArea[n];
         lg_ch_peak_phd[i][n] = evt->ODLGPulses[i].chPeakAmp[n];
         lg_ch_ID[i][n]       = evt->ODLGPulses[i].chID[n];
         lg_ch_PeakTime[i][n] = evt->ODLGPulses[i].chPeakTime[n];
         lg_ch_Saturated[i][n] = evt->ODLGPulses[i].chSaturated[n];
      }
    }    
  }

  //------------------------------------------------
  // RQ from HGPulses and LGPulses  
  //------------------------------------------------
  int nHGPulses, nLGPulses;
  vector<float> HGPeakAmp, HGPeakTime, HGArea, HGrmsWidth;
  vector<float> LGPeakAmp, LGPeakTime, LGArea, LGrmsWidth;
  //Channel pulse parameters
  vector<vector<float>> hg_ch_area_phd, hg_ch_peak_phd, hg_ch_ID, hg_ch_PeakTime, hg_ch_Saturated;
  vector<vector<float>> lg_ch_area_phd, lg_ch_peak_phd, lg_ch_ID, lg_ch_PeakTime, lg_ch_Saturated;

  
  //------------------------------------------------
  // RQ from singleScatters
  //------------------------------------------------
  bool isInteraction;
  float s1, s2, s1c, s2c;
  float x,y, R2, z,driftTime;


  
};

/**
 * # Some use full functions 
 *
 * ### Description: 
 *  - `Loggify(TAxis* axis)` : Convert the linear bining of the axis into a log-scale bining
 *  - `LoggifyX(TH1* h)`     : Convert in log-scale the X axis of the TH1
 *  - `LoggifyXY(TH2* h)`    : Convert in log-scale the X and Y axis of the TH2
 *  - `LoggifyX(TH2* h)`     : Convert in log-scale the X axis of the TH2
 *  - `LoggifyY(TH2* h)`     : Convert in log-scale the Y axis of the TH2
 */
void Loggify(TAxis* axis) {
  int bins = axis->GetNbins();

  Axis_t from = TMath::Log10(axis->GetXmin());
  Axis_t to = TMath::Log10(axis->GetXmax());
  Axis_t width = (to - from) / bins;
  Axis_t *new_bins = new Axis_t[bins + 1];
  
  for (int i = 0; i <= bins; i++) new_bins[i] = TMath::Power(10, from + i * width);
  axis->Set(bins, new_bins); 
  delete[] new_bins; 
}

void LoggifyX(TH1* h)  { Loggify(h->GetXaxis()); }
void LoggifyXY(TH2* h) { Loggify(h->GetXaxis()); Loggify(h->GetYaxis()); }
void LoggifyX(TH2* h)  { Loggify(h->GetXaxis()); }
void LoggifyY(TH2* h)  { Loggify(h->GetYaxis()); }

/**
 * ## Definition of the cuts <a name="cuts"></a>
 * This code block contains the definition of all the cuts.
 *
 * ### Description: 
 * Cuts on the high gain based RQs
 * Need to look at OD events to see what kind of cuts are sensible.
 * 
 */

/**
 * ## Main executable <a name="main"></a>
 *
 * Main executable
 *
 * ## Arguments:
 * - `txtFileList`: Name of the file that contains the list of file to process (default: "RQfile.list") 
 */

void speOD(TString txtFileList = "RQfile.list")
{
  
  //------------------------------------------------
  // Prevent canvases from being drawn.
  //------------------------------------------------
  gROOT->SetBatch(kTRUE);

  //------------------------------------------------
  // Create and start a timer
  //------------------------------------------------  
  TStopwatch* clock = new TStopwatch();
  clock->Start();

  //------------------------------------------------
  // Create the output file. If the file already
  // exist: overwrite on it ("recreate" option)
  //------------------------------------------------
  outfile = new TFile(outname, "recreate");
  cout << "Writing output to: "<<outfile->GetName()<<endl;

  //------------------------------------------------
  // Creation of the TChain and set the TTree Events
  // as the tree to extract from the file that we
  // are going to load
  //------------------------------------------------
  TChain* events = new TChain("Events", "Events");
  
  //------------------------------------------------
  // Load all the TTRee Events in the TChain from
  // all the files listed in txtFileList
  //------------------------------------------------
  load_chain(txtFileList, events);

  //------------------------------------------------
  // 1) Create all the histrograms
  // 2) Loop over all the events in the TChain
  // 3) Fill all the histrograms according some cuts
  //------------------------------------------------
  cout << "Begin event loop"<<endl;
  eventLoop(events);
  cout << "Event event loop\n"<<endl;

  //------------------------------------------------
  // Write all the histrograms in the ROOT file and
  // close it
  //------------------------------------------------
  outfile->Write();
  outfile->Close();
  
  cout << "Done!"<<" "<<clock->RealTime()<<" s."<<endl;

  delete events;
  delete clock;
  
}

/**
 * ## Load chain <a name="load"></a>
 *
 * Open a set of ROOT files specified in txt file list
 * ### Arguments:
 * - `txtFileList`: Name of the file that contains the list of file to process
 * - `chain`: Pointer to the TChain
 *
 */
void load_chain(TString txtFileList, TChain* chain){
  
  cout << "Loading file names from "<<txtFileList << " into "<<chain->GetName()<<endl;
  
  //------------------------------------------------
  // Open the txtFileList
  //------------------------------------------------
  ifstream fileList(txtFileList);
  string file;

  //------------------------------------------------
  // Check if the txtFileList exists, exit if not
  //------------------------------------------------
  if (fileList.is_open()) {
    //------------------------------------------------
    // Loop over the txtFileList
    //------------------------------------------------    
    while ( getline(fileList, file) ) {
      //------------------------------------------------
      // Add the current file in the TChain
      //------------------------------------------------
      chain->AddFile(file.c_str());
    }
    //------------------------------------------------
    // Close the txtFileList
    //------------------------------------------------
    fileList.close();
  }else{
    cout<<"The file "<< txtFileList <<" doesn't exist. Exiting !!"<<endl;
    exit(-1);
  }
}



/**
 * ## Event loop <a name="loop"></a>
 *
 * ### Description: 
 * This function creates all the historgrams, loop over all the events loaded on the TChain and FIll all the histograms 
 *
 * ### Arguments:
 *  - `events`: TChain with all the files from the txtFileList loaded
 * 
 */
void eventLoop(TChain* events)
{

  //------------------------------------------------
  // Creation of the RQEvent event class 
  //------------------------------------------------
  RQEvent* evt = new RQEvent();
  
  //------------------------------------------------
  // Link the pointer of the event and the TChain
  // aka, each ecent will be loaded in evt
  //------------------------------------------------
  events->SetBranchAddress("RQEvent", &evt);

  //------------------------------------------------
  // Creation of the lite event structure
  //------------------------------------------------
  MyEvent* myEvt = new MyEvent();

  //------------------------------------------------
  // Preparation of the output
  //------------------------------------------------
  // Set the output file as the 
  outfile->cd();

  size_t ODPmtCount = 120;
  std::vector<std::vector<float>> HGChArea;
  std::vector<std::vector<float>> HGChAmplitude;
  std::vector<std::vector<float>> LGChArea;
  std::vector<std::vector<float>> LGChAmplitude;
  HGChArea.resize(ODPmtCount);
  HGChAmplitude.resize(ODPmtCount);
  LGChArea.resize(ODPmtCount);
  LGChAmplitude.resize(ODPmtCount);

  // Definition of the histogram properties, this is not the definition of the cuts !!
  int Nbins_ch_area   = 100;   float min_ch_area   = 0;   float max_ch_area = 20;
  int Nbins_ch_height =  50;   float min_ch_height = 0;   float max_ch_height = 10;        
  
  //------------------------------------------------
  // Definition of 1D histograms
  // TH1F("name", "title; Xaxis; Yaxis", nbinsx, xlow, xup)
  //------------------------------------------------
  // high gain pulses
  TH1F* h_nHGPulses = new TH1F("h_nHGPulses", "; nHGPulses", 50, 0, 50);
  TH1F* h_HG_ch_area   = new TH1F("h_HG_ch_area", "; hg_ch_area_phd", Nbins_ch_area, min_ch_area, max_ch_area);
  TH1F* h_HG_ch_height = new TH1F("h_HG_ch_height", "; hg_ch_peak_phd", Nbins_ch_height, min_ch_height, max_ch_area);

  // low gain pulses
  TH1F* h_nLGPulses = new TH1F("h_nLGPulses", "; nLGPulses", 50, 0, 50);
  TH1F* h_LG_ch_area   = new TH1F("h_LG_ch_area", "; LG_ch_area_phd", Nbins_ch_area, min_ch_area, max_ch_area);
  TH1F* h_LG_ch_height = new TH1F("h_LG_ch_height", "; LG_ch_peak_phd", Nbins_ch_height, min_ch_height, max_ch_area);

  //Array of histograms for individual channels
  TH1F* h_HG_Channel_Area[ODPmtCount];
  TH1F* h_LG_Channel_Area[ODPmtCount];

  char* histname = new char[11];
  for (int i = 0; i < 120; ++i) {
     int HGChannelNumber =  801 + i;
     int LGChannelNumber = 1801 + i;
     sprintf(histname, "h_area_%d",HGChannelNumber);
     h_HG_Channel_Area[i] = new TH1F(histname,"; hg_ch_area_phd",Nbins_ch_area, min_ch_area, max_ch_area);
     sprintf(histname, "h_area_%d",LGChannelNumber);
     h_LG_Channel_Area[i] = new TH1F(histname,"; lg_ch_area_phd",Nbins_ch_area, min_ch_area, max_ch_area);
  }
  
  //------------------------------------------------
  // Definition of 2D histograms
  // TH2F("name", "title; Xaxis; Yaxis", nbinsx, xlow, xup,  nbinsy, ylow, yup)
  //------------------------------------------------
  // high gain pulses
  TH2F* h2_HG_area_height = new TH2F("h2_HG_area_height", "", Nbins_ch_area, min_ch_area, max_ch_area, Nbins_ch_height, min_ch_height, max_ch_height);

  // low gain pulses
  TH2F* h2_LG_area_height = new TH2F("h2_LG_area_height", "", Nbins_ch_area, min_ch_area, max_ch_area, Nbins_ch_height, min_ch_height, max_ch_height);
  
  //------------------------------------------------
  // Main event loop
  //------------------------------------------------

  // Count the total number of event in the TChain
  const Int_t nevents = events->GetEntries();

  // Loop over all the events
  for (Int_t n=0; n<nevents; ++n) {
     if (n%1000 == 0) cout << "Processing "<< n << "/"<<nevents<<endl;

    //------------------------------------------------    
    // Load the nth event in the RQEvent class,
    // aka set all the RQs in evt
    //------------------------------------------------
    events->GetEntry(n);
    
    //------------------------------------------------
    // build some variables on the fly
    //------------------------------------------------
    myEvt->SetVariables(evt);
    
    //------------------------------------------------
    // create booleans for the cuts
    //------------------------------------------------
    
    //------------------------------------------------
    // fill histograms with and without cuts application
    //------------------------------------------------

    // High gain
    h_nHGPulses->Fill(myEvt->nHGPulses);

    for (int i = 0; i < myEvt->nHGPulses; ++i) {
      for (size_t k = 0; k < myEvt->hg_ch_area_phd[i].size(); ++k) {
        int idx = (myEvt->hg_ch_ID[i][k] - 801); //801 is idx 0
        //Probably want some cuts here...
        if(myEvt->hg_ch_area_phd[i][k] != 0) {
          h_HG_Channel_Area[idx]->Fill(myEvt->hg_ch_area_phd[i][k]);
          //Fill histograms for all channels
          h_HG_ch_area      -> Fill(myEvt->hg_ch_area_phd[i][k]);
          h_HG_ch_height    -> Fill(myEvt->hg_ch_peak_phd[i][k]);
          h2_HG_area_height -> Fill(myEvt->hg_ch_area_phd[i][k], myEvt->hg_ch_peak_phd[i][k]);
        }
      }
    }

    // Low gain
    h_nLGPulses->Fill(myEvt->nLGPulses);

    for (int i = 0; i < myEvt->nLGPulses; ++i) {
      for (size_t k = 0; k < myEvt->lg_ch_area_phd[i].size(); ++k) {
        int idx = (myEvt->lg_ch_ID[i][k] - 1801); //1801 is idx 0
        //Probably want some cuts here...
        if(myEvt->lg_ch_area_phd[i][k] != 0) {
          h_LG_Channel_Area[idx]->Fill(myEvt->lg_ch_area_phd[i][k]);
          //Fill histograms for all channels
          h_LG_ch_area      -> Fill(myEvt->lg_ch_area_phd[i][k]);
          h_LG_ch_height    -> Fill(myEvt->lg_ch_peak_phd[i][k]);
          h2_LG_area_height -> Fill(myEvt->lg_ch_area_phd[i][k], myEvt->lg_ch_peak_phd[i][k]);
        }
      }
    }
    
  }

  //--------------------------------------------------
  // Calculate mean and standard deviation per channel
  //--------------------------------------------------

  TH1F* h_mean_HG_SPEArea = new TH1F("h_mean_HG_SPEArea", "; mean HG SPE area [phd]", 100, 0, 3);
  TH1F* h_error_HG_SPEArea = new TH1F("h_error_HG_SPEArea", "; error on mean HG SPE area [phd]", 10, 0, 0.1);
  TH1F* h_mean_LG_SPEArea = new TH1F("h_mean_LG_SPEArea", "; mean LG SPE area [phd]", 100, 0, 3);
  TH1F* h_error_LG_SPEArea = new TH1F("h_error_LG_SPEArea", "; error on mean LG SPE area [phd]", 10, 0, 0.1);

  Double_t meanHGSPE[ODPmtCount];
  Double_t errorHGSPE[ODPmtCount];
  Double_t HGChannelNumber[ODPmtCount];
  Double_t empty[ODPmtCount];
  Double_t meanLGSPE[ODPmtCount];
  Double_t errorLGSPE[ODPmtCount];
  Double_t LGChannelNumber[ODPmtCount];

  std::cout<< "OD SPE Areas: "<<std::endl;
  for (size_t idx = 0; idx < ODPmtCount; ++idx) {
    HGChannelNumber[idx] = idx +  801;
    LGChannelNumber[idx] = idx + 1801;
    empty[idx]     = 0;
    meanHGSPE[idx]   = h_HG_Channel_Area[idx]->GetMean(1);
    meanLGSPE[idx]   = h_LG_Channel_Area[idx]->GetMean(1);
    errorHGSPE[idx]  = h_HG_Channel_Area[idx]->GetMeanError(1);
    errorLGSPE[idx]  = h_LG_Channel_Area[idx]->GetMeanError(1);
    h_mean_HG_SPEArea->Fill(meanHGSPE[idx]);
    h_mean_LG_SPEArea->Fill(meanLGSPE[idx]);
    h_error_HG_SPEArea->Fill(errorHGSPE[idx]);
    h_error_LG_SPEArea->Fill(errorLGSPE[idx]);
    std::cout << "PMT " << HGChannelNumber[idx] << " HG " << meanHGSPE[idx] << " +/- " << errorHGSPE[idx] << " LG " << meanLGSPE[idx] << " +/- " << errorLGSPE[idx] << std::endl;
  }

   TGraphErrors* g_hg_spe_per_pmt = new TGraphErrors(ODPmtCount,HGChannelNumber,meanHGSPE,empty,errorHGSPE);
   g_hg_spe_per_pmt->SetTitle("OD HG SPE Area per PMT");
   g_hg_spe_per_pmt->SetName("hg_spe_per_pmt");
   g_hg_spe_per_pmt->GetXaxis()->SetTitle("Channel Number");
   g_hg_spe_per_pmt->GetYaxis()->SetTitle("SPE Area HG [phd]");
   g_hg_spe_per_pmt->Write();

   TGraphErrors* g_lg_spe_per_pmt = new TGraphErrors(ODPmtCount,LGChannelNumber,meanLGSPE,empty,errorLGSPE);
   g_lg_spe_per_pmt->SetTitle("OD LG SPE Area per PMT");
   g_lg_spe_per_pmt->SetName("lg_spe_per_pmt");
   g_lg_spe_per_pmt->GetXaxis()->SetTitle("Channel Number");
   g_lg_spe_per_pmt->GetYaxis()->SetTitle("SPE Area LG [phd]");
   g_lg_spe_per_pmt->Write();

  //------------------------------------------------
  // end event loop
  //------------------------------------------------
  
}

