/**
 * # Macro to plot output from speOD macro.
 * Adapted tutorial macro by QR (25/04/2018, TF)
 * This macro reads all the files listed in the file `RQfile.list` and generate an output rootfile `macro.root`.
 * ## Usage: 
 * Run the plot macro:
 * ```bash
 * # In compilated mode:
 * root plot_root.C+ 
 * # or in interpreted mode:
 * root plot_root.C
 * ```
 *
 *
 * The macro will creat and display the figure. In this macro, you can do all the treatment that you want on the style of the figure, scaling, comparison, simple calculation, ...
 *
 *---
 *# Description of the macro
 * 
 * In the following section, each part of the macro will be described. The major blocks are: 
 * 1. [Headers] (#Headers)
 * 2. [Main executable] (#main)
 *
 */

/**
 * ## Headers <a name="Headers"></a>
 * Include all the headers
 *
 */
// Standard C++ headers
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

// ROOT Headers
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1D.h"
#include "THStack.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TMinuit.h"
#include "TColor.h"
#include "TLine.h"
#include "TLatex.h"
#include "TSystem.h"

// STD namespace
using namespace std;



/**
 * ## Main executable <a name="main"></a>
 *
 * Main executable
 *
 * 
 */
void plot_speOD(){


  /**
   * ROOT file opening
   */
  TFile* f = TFile::Open("speOD.root");
  
  /**
   * Get the result histograms and TGraphErrors from file
   * When you use the method f->Get(char* name), ROOT return a TObject. In order to manipulate it, you have to cas it as a TH1F.
   */  

  TH1F* h_mean_HG_SPEArea  = (TH1F*)f->Get("h_mean_HG_SPEArea");
  TH1F* h_error_HG_SPEArea = (TH1F*)f->Get("h_error_HG_SPEArea");
  TH1F* h_HG_ch_area    = (TH1F*)f->Get("h_HG_ch_area");
  TGraphErrors* g_hg_spe_per_pmt = (TGraphErrors*)f->Get("hg_spe_per_pmt");

  TH1F* h_mean_LG_SPEArea  = (TH1F*)f->Get("h_mean_LG_SPEArea");
  TH1F* h_error_LG_SPEArea = (TH1F*)f->Get("h_error_LG_SPEArea");
  TH1F* h_LG_ch_area    = (TH1F*)f->Get("h_LG_ch_area");
  TGraphErrors* g_lg_spe_per_pmt = (TGraphErrors*)f->Get("lg_spe_per_pmt");
  
  /**
   * Remove the stat box
   */
  h_mean_HG_SPEArea->SetStats(0);
  h_mean_LG_SPEArea->SetStats(0);
  h_error_HG_SPEArea->SetStats(0);
  h_error_LG_SPEArea->SetStats(0);
  
  /**
   * Modify the style of the histogram
   */

  h_mean_HG_SPEArea->SetXTitle("Mean HG SPE Area [phd]");
  h_mean_HG_SPEArea->SetYTitle("count");
  
  h_mean_LG_SPEArea->SetXTitle("Mean LG SPE Area [phd]");
  h_mean_LG_SPEArea->SetYTitle("count");

  h_error_HG_SPEArea->SetXTitle("Error on HG SPE Area [phd]");
  h_error_HG_SPEArea->SetYTitle("count");
  
  h_error_LG_SPEArea->SetXTitle("Error on LG SPE Area [phd]");
  h_error_LG_SPEArea->SetYTitle("count");

  h_HG_ch_area->SetTitle("All HG Channels");
  h_HG_ch_area->SetXTitle("Channel Pulse SPE Area [phd]");
  h_HG_ch_area->SetYTitle("count");
  
  h_LG_ch_area->SetTitle("All LG Channels");
  h_LG_ch_area->SetXTitle("Channel Pulse SPE Area [phd]");
  h_LG_ch_area->SetYTitle("count");
  
  /**
   * ## Draw the figures
   */  
  
  TCanvas * c;
  
  // Create a new canvas with all the default arguments  
  c = new TCanvas;
  h_mean_HG_SPEArea->Draw("");
  c->SaveAs("plot/OD_HG_mean_speArea.png");
  c = new TCanvas;
  h_mean_LG_SPEArea->Draw("");
  c->SaveAs("plot/OD_LG_mean_speArea.png");

  c = new TCanvas;
  h_error_HG_SPEArea->Draw("");
  c->SaveAs("plot/OD_HG_error_speArea.png");
  c = new TCanvas;
  h_error_LG_SPEArea->Draw("");
  c->SaveAs("plot/OD_LG_error_speArea.png");

  c = new TCanvas;
  h_HG_ch_area->Draw("");
  c->SaveAs("plot/OD_HG_allChannels.png");
  c = new TCanvas;
  h_LG_ch_area->Draw("");
  c->SaveAs("plot/OD_LG_allChannels.png");

  c = new TCanvas;
  g_hg_spe_per_pmt->Draw("ap");
  g_hg_spe_per_pmt->GetXaxis()->SetRange(800,920);
  g_hg_spe_per_pmt->GetYaxis()->SetRange(0,3);
  g_hg_spe_per_pmt->GetYaxis()->SetTitle("SPE Area [phd]");
  c->SaveAs("plot/OD_HG_spe_per_pmt.png");

  c = new TCanvas;
  g_lg_spe_per_pmt->Draw("ap");
  g_lg_spe_per_pmt->GetXaxis()->SetRange(1800,1920);
  g_lg_spe_per_pmt->GetYaxis()->SetRange(0,3);
  g_lg_spe_per_pmt->GetYaxis()->SetTitle("SPE Area [phd]");
  c->SaveAs("plot/OD_LG_spe_per_pmt.png");

  std::cout<<"After drawing"<<std::endl;

  // overlay
  int n       = g_lg_spe_per_pmt->GetN();
  Double_t* x = g_lg_spe_per_pmt->GetX();
  Double_t* y = g_lg_spe_per_pmt->GetY();
  Double_t* xError = g_lg_spe_per_pmt->GetEX();
  Double_t* yError = g_lg_spe_per_pmt->GetEY();
  for (int i = 0; i < n; ++i) {
    x[i] -= 1000;
  }
  
  c = new TCanvas;
  TGraphErrors* g_lg_spe_overlay = new TGraphErrors(n,x,y,xError,yError);
  g_lg_spe_per_pmt->SetName("Low Gain");
  g_lg_spe_overlay->GetXaxis()->SetTitle("PMT Number");
  g_lg_spe_overlay->GetXaxis()->SetRange(800,920);
  g_lg_spe_overlay->GetYaxis()->SetTitle("SPE Area [phd]");
  g_lg_spe_overlay->GetYaxis()->SetRange(0, 3);
  g_lg_spe_overlay->SetMarkerColor(2);
  g_lg_spe_overlay->Draw("ap");
  g_hg_spe_per_pmt->SetMarkerColor(4);
  g_hg_spe_per_pmt->SetName("High Gain");
  g_hg_spe_per_pmt->Draw("p");

  /*TLegend* legend = new TLegend(0.1, 2.7, 0.48, 0.9);
  legend->AddEntry(g_hg_spe_per_pmt, "HG");
  legend->AddEntry(g_lg_spe_overlay, "LG");
  legend->Draw(); */
  c->SaveAs("plot/OD_LG_HG_overlay.png");
  /**
   * ### End of the macro
   */  

} 
