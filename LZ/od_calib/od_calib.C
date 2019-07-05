#define od_calib_cxx

#include "od_calib.h"
#include <TH2.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>

#include <iostream>
#include <numeric>

using namespace std;

// Function declarations
void deriveDetRQs(MyEvent & e, MyDetectorPulses & defdet, MyDetectorPulses & newdet);
void deriveMctRQs(MyEvent & e);
void buildSS(MyEvent & e);
void buildMS(MyEvent & e);
bool sortinrev(const pair<float,int> &a, 
               const pair<float,int> &b);
bool comp_pair(const pair<float,int> &a, 
               const pair<float,int> &b);
pair < MyDetectorPulses, int > det(MyEvent & e, int p);

// TPC geometry
static const float kTPCradius = 145.6/2.; //[cm]
static const float kTPCheight = 145.6;    //[cm]

static const int nPulseClasses = 8;
enum PulseClass : unsigned int
{
  OTHER,
  OTHERS2,
  SE,
  SPE,
  MPE,
  S1,
  S2,
  ALL
};

const vector<PulseClass> allPulseClasses()
{
  vector<PulseClass> allClasses;
  allClasses.push_back(PulseClass::OTHER);
  allClasses.push_back(PulseClass::OTHERS2);
  allClasses.push_back(PulseClass::SE);
  allClasses.push_back(PulseClass::SPE);
  allClasses.push_back(PulseClass::MPE);
  allClasses.push_back(PulseClass::S1);
  allClasses.push_back(PulseClass::S2);
  allClasses.push_back(PulseClass::ALL);
  return allClasses;
}

const TString pulseClassToString(PulseClass clazz)
{
  if      (clazz == PulseClass::OTHER)   return "Other";
  else if (clazz == PulseClass::OTHERS2) return "OtherS2";
  else if (clazz == PulseClass::SE)      return "SE";
  else if (clazz == PulseClass::SPE)     return "SPE";
  else if (clazz == PulseClass::MPE)     return "MPE";
  else if (clazz == PulseClass::S1)      return "S1";
  else if (clazz == PulseClass::S2)      return "S2";
  else if (clazz == PulseClass::ALL)     return "All";
  else return "None";    
}

//________________________________________________________
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

//________________________________________________________
void od_calib::Begin(TTree * /*tree*/)
{
  TString option = GetOption();
}

//________________________________________________________
void od_calib::SlaveBegin(TTree * /*tree*/)
{

  TString option = GetOption();

  e = MyEvent();

  h1_all_s1_s2 = new TH1F("h1_all_s1_s2", "; S1 [phd] S2 [phd]", 10000, 1, 5e5);
  fOutput->Add(h1_all_s1_s2);

  h2_all_s1_s2 = new TH2F("h2_all_s1_s2", "; S1 [phd]; S2 [phd]", 500, 1e2, 5e4, 500, 1e2, 1e7);
  LoggifyXY(h2_all_s1_s2);
  fOutput->Add(h2_all_s1_s2);

  h1_rate = new TH1F("h1_rate", "; rate [Hz]", 10,0,10);
  fOutput->Add(h1_rate);

  //bkbl

  h2_all_subS2maxS2ratio_h2w = new TH2F("h2_all_subS2maxS2ratio_h2w", "; subS2 area / maxS2 area; h2w [phd/ns/ns]", 500, 5e-5, 1.5, 500, 1e-5, 10);
  LoggifyXY(h2_all_subS2maxS2ratio_h2w);
  fOutput->Add(h2_all_subS2maxS2ratio_h2w);
  
  h1_tpcLG_nPulses=new TH1F("h1_tpcLG_nPulses", "h1_tpcLG_nPulses", 50, 0, 50);
  fOutput->Add(h1_tpcLG_nPulses);
  h1_tpcLG_pulseStartTime=new TH1F("h1_tpcLG_pulseStartTime", "h1_tpcLG_pulseStartTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_tpcLG_pulseStartTime);
  h1_tpcLG_pulseEndTime=new TH1F("h1_tpcLG_pulseEndTime", "h1_tpcLG_pulseEndTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_tpcLG_pulseEndTime);
  h1_tpcLG_pulseLength=new TH1F("h1_tpcLG_pulseLength", "h1_tpcLG_pulseLength ms" , 100, 0, 5);
  fOutput->Add(h1_tpcLG_pulseLength);
  h1_tpcLG_pulseArea=new TH1F("h1_tpcLG_pulseArea", "h1_tpcLG_pulseArea" , 100, 0, 10);
  fOutput->Add(h1_tpcLG_pulseArea);
  h1_tpcLG_photonCount=new TH1F("h1_tpcLG_photonCount", "h1_tpcLG_photonCount" , 100, 0, 10);
  fOutput->Add(h1_tpcLG_photonCount);
  h1_tpcLG_peakAmp=new TH1F("h1_tpcLG_peakAmp", "h1_tpcLG_peakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_tpcLG_peakAmp);
  h1_tpcLG_peakTime=new TH1F("h1_tpcLG_peakTime", "h1_tpcLG_peakTime", 1000, 0, 1000);
  fOutput->Add(h1_tpcLG_peakTime);
  h1_tpcLG_nPulsesSat=new TH1F("h1_tpcLG_nPulsesSat", "h1_tpcLG_nPulsesSat" , 20, 0, 20);
  fOutput->Add(h1_tpcLG_nPulsesSat);
  h1_tpcLG_chPulseArea=new TH1F("h1_tpcLG_chPulseArea", "h1_tpcLG_chPulseArea" , 100, 0, 10);
  fOutput->Add(h1_tpcLG_chPulseArea);
  h1_tpcLG_chPulseAreaSize=new TH1F("h1_tpcLG_chPulseAreaSize", "h1_tpcLG_chPulseAreaSize" , 140, 0, 140);
  fOutput->Add(h1_tpcLG_chPulseAreaSize);
  h1_tpcLG_chPeakTime=new TH1F("h1_tpcLG_chPeakTime", "h1_tpcLG_chPeakTime" , 1000, 0, 1000);
  fOutput->Add(h1_tpcLG_chPeakTime);
  h1_tpcLG_chPeakAmp=new TH1F("h1_tpcLG_chPeakAmp", "h1_tpcLG_chPeakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_tpcLG_chPeakAmp);
  h2_tpcLG_pulseArea_pulseLength=new TH2F("h2_tpcLG_pulseArea_pulseLength","h2_tpcLG_pulseArea_pulseLength", 20, 0, 20, 100, 0, 10);
  fOutput->Add(h2_tpcLG_pulseArea_pulseLength);
  h2_tpcLG_pulseArea_peakAmp=new TH2F("h2_tpcLG_pulseArea_peakAmp","h2_tpcLG_pulseArea_peakAmp", 20, 0, 20, 50, 0, 0.3);
  fOutput->Add(h2_tpcLG_pulseArea_peakAmp);

    
  h1_odLG_nPulses=new TH1F("h1_odLG_nPulses", "h1_odLG_nPulses", 50, 0, 50);
  fOutput->Add(h1_odLG_nPulses);
  h1_odLG_pulseStartTime=new TH1F("h1_odLG_pulseStartTime", "h1_odLG_pulseStartTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_odLG_pulseStartTime);
  h1_odLG_pulseEndTime=new TH1F("h1_odLG_pulseEndTime", "h1_odLG_pulseEndTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_odLG_pulseEndTime);
  h1_odLG_pulseLength=new TH1F("h1_odLG_pulseLength", "h1_odLG_pulseLength ms" , 100, 0, 5);
  fOutput->Add(h1_odLG_pulseLength);
  h1_odLG_pulseArea=new TH1F("h1_odLG_pulseArea", "h1_odLG_pulseArea" , 100, 0, 10);
  fOutput->Add(h1_odLG_pulseArea);
  h1_odLG_photonCount=new TH1F("h1_odLG_photonCount", "h1_odLG_photonCount" , 100, 0, 10);
  fOutput->Add(h1_odLG_photonCount);
  h1_odLG_peakAmp=new TH1F("h1_odLG_peakAmp", "h1_odLG_peakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_odLG_peakAmp);
  h1_odLG_peakTime=new TH1F("h1_odLG_peakTime", "h1_odLG_peakTime", 1000, 0, 1000);
  fOutput->Add(h1_odLG_peakTime);
  h1_odLG_nPulsesSat=new TH1F("h1_odLG_nPulsesSat", "h1_odLG_nPulsesSat" , 20, 0, 20);
  fOutput->Add(h1_odLG_nPulsesSat);
  h1_odLG_chPulseArea=new TH1F("h1_odLG_chPulseArea", "h1_odLG_chPulseArea" , 100, 0, 10);
  fOutput->Add(h1_odLG_chPulseArea);
  h1_odLG_chPulseAreaSize=new TH1F("h1_odLG_chPulseAreaSize", "h1_odLG_chPulseAreaSize" , 140, 0, 140);
  fOutput->Add(h1_odLG_chPulseAreaSize);
  h1_odLG_chPeakTime=new TH1F("h1_odLG_chPeakTime", "h1_odLG_chPeakTime" , 1000, 0, 1000);
  fOutput->Add(h1_odLG_chPeakTime);
  h1_odLG_chPeakAmp=new TH1F("h1_odLG_chPeakAmp", "h1_odLG_chPeakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_odLG_chPeakAmp);
  h2_odLG_pulseArea_pulseLength=new TH2F("h2_odLG_pulseArea_pulseLength","h2_odLG_pulseArea_pulseLength", 20, 0, 20, 100, 0, 10);
  fOutput->Add(h2_odLG_pulseArea_pulseLength);
  h2_odLG_pulseArea_peakAmp=new TH2F("h2_odLG_pulseArea_peakAmp","h2_odLG_pulseArea_peakAmp", 20, 0, 20, 50, 0, 0.3);
  fOutput->Add(h2_odLG_pulseArea_peakAmp);

  h1_odHG_nPulses=new TH1F("h1_odHG_nPulses", "h1_odHG_nPulses", 50, 0, 50);
  fOutput->Add(h1_odHG_nPulses);
  h1_odHG_pulseStartTime=new TH1F("h1_odHG_pulseStartTime", "h1_odHG_pulseStartTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_odHG_pulseStartTime);
  h1_odHG_pulseEndTime=new TH1F("h1_odHG_pulseEndTime", "h1_odHG_pulseEndTime ns" , 100000, 0, 2000000);
  fOutput->Add(h1_odHG_pulseEndTime);
  h1_odHG_pulseLength=new TH1F("h1_odHG_pulseLength", "h1_odHG_pulseLength ms" , 100, 0, 5);
  fOutput->Add(h1_odHG_pulseLength);
  h1_odHG_pulseArea=new TH1F("h1_odHG_pulseArea", "h1_odHG_pulseArea" , 100, 0, 10);
  fOutput->Add(h1_odHG_pulseArea);
  h1_odHG_photonCount=new TH1F("h1_odHG_photonCount", "h1_odHG_photonCount" , 100, 0, 10);
  fOutput->Add(h1_odHG_photonCount);
  h1_odHG_peakAmp=new TH1F("h1_odHG_peakAmp", "h1_odHG_peakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_odHG_peakAmp);
  h1_odHG_peakTime=new TH1F("h1_odHG_peakTime", "h1_odHG_peakTime", 1000, 0, 1000);
  fOutput->Add(h1_odHG_peakTime);
  h1_odHG_nPulsesSat=new TH1F("h1_odHG_nPulsesSat", "h1_odHG_nPulsesSat" , 20, 0, 20);
  fOutput->Add(h1_odHG_nPulsesSat);
  h1_odHG_chPulseArea=new TH1F("h1_odHG_chPulseArea", "h1_odHG_chPulseArea" , 100, 0, 10);
  fOutput->Add(h1_odHG_chPulseArea);
  h1_odHG_chPulseAreaSize=new TH1F("h1_odHG_chPulseAreaSize", "h1_odHG_chPulseAreaSize" , 140, 0, 140);
  fOutput->Add(h1_odHG_chPulseAreaSize);
  h1_odHG_chPeakTime=new TH1F("h1_odHG_chPeakTime", "h1_odHG_chPeakTime" , 1000, 0, 1000);
  fOutput->Add(h1_odHG_chPeakTime);
  h1_odHG_chPeakAmp=new TH1F("h1_odHG_chPeakAmp", "h1_odHG_chPeakAmp" , 50, 0, 0.3);
  fOutput->Add(h1_odHG_chPeakAmp);
  h2_odHG_pulseArea_pulseLength=new TH2F("h2_odHG_pulseArea_pulseLength","h2_odHG_pulseArea_pulseLength", 20, 0, 20, 100, 0, 10);
  fOutput->Add(h2_odHG_pulseArea_pulseLength);
  h2_odHG_pulseArea_peakAmp=new TH2F("h2_odHG_pulseArea_peakAmp","h2_odHG_pulseArea_peakAmp", 20, 0, 20, 50, 0, 0.3);
  fOutput->Add(h2_odHG_pulseArea_peakAmp);

}

//________________________________________________________
// Define cuts
bool CXTPC(MyEvent const& e) { return e.tpcHG.nPulses > 0; }
bool CXnS1only(MyEvent const& e) {
  bool isS1only = true;
  for (int p=0; p<e.tpcHG.nPulses; ++p) {
    if (e.tpcHG.s2prob[p]==1)
      isS1only = false;
  }
  return !isS1only;
}

bool CXOtherS2notBiggest (MyEvent const& e) { 
  if (e.tpc.maxPulseClass == PulseClass::OTHERS2) return false;
  return true; }

bool CXPPT(MyEvent const& e) {
  for (size_t p=0; p<e.mcPulseLength.size(); ++p) {
    if (e.mcPulseLength[p] > 1e6)
      return false;
  }
  return true;
}

bool CXMC4MeV(MyEvent const& e) {
  for (size_t v=0; v<e.mcVtxEnergyDep.size(); ++v) {
    if (e.mcVtxEnergyDep[v] > 4000 && (e.mcVtxVolumeName[v] == "LiquidXenonTarget" ||
				       e.mcVtxVolumeName[v] == "LiquidSkinXenon" ||
				       e.mcVtxVolumeName[v] == "ReverseFieldRegion" ||
				       e.mcVtxVolumeName[v] == "LiquidSkinXenonBank")) {
      return false;
    }
    if (e.mcVtxVolumeName[v] == "LiquidXenonTarget" &&
	e.mcVtxRawS2photons[v] == 0) {
      return false;
    }
  }
  return true;
}

bool CXHE(MyEvent const& e) { return e.tpc.maxS1area > 150; }
bool CXLE(MyEvent const& e) { return (e.tpc.maxS1area > 0 && e.tpc.maxS1area < 600); }
bool CXS1first(MyEvent const& e) {
  // check that there is an S1 (otherwise, this cut makes no sense)
  if (e.tpc.maxS1pulseID == -1) return false;
  // check that s1pulseID < s2pulseID
  if (e.tpc.maxS2pulseID != -1 && e.tpc.maxS1pulseID > e.tpc.maxS2pulseID) return false;
  // No S2, SE, OtherS2 allowed before max S1
  for (int p=0; p<e.tpc.maxS1pulseID; ++p) {
    if (e.tpc.pulseClass[p] == PulseClass::S2 ||
        e.tpc.pulseClass[p] == PulseClass::SE ||
        e.tpc.pulseClass[p] == PulseClass::OTHERS2 ||
	e.tpc.pulseClass[p] == PulseClass::OTHER)
      return false;
  }
  return true;
}

// Single scatter cut
bool CXSSsubS2H2W(MyEvent const& e, int n) {
  if (e.tpc.maxS1pulseID<0 || e.tpc.maxS2pulseID<0) return false; // If there is no S1 or S2, it is not any kind of scatter
  if (e.tpc.maxS2pulseID > -1 && e.tpc.subS2pulses[n].second<0) return true; // If there is a first S2, but no second S2, it must be a single scatter
  const float x1 = .14; //1e-1; //.14;
  const float x2 = 5.5e-3; //3e-2; //5.5e-3;
  const float y1 = 1e-5; //5e-4; //1e-5;
  const float y2 = 10; //4e-2; //10;
  const float logx1 = TMath::Log10(x1);
  const float logx2 = TMath::Log10(x2);
  const float logy1 = TMath::Log10(y1);
  const float logy2 = TMath::Log10(y2);
  const float m = (logy2-logy1)/(logx2-logx1);
  const float b = logy1-m*logx1;
  const float xx = e.tpc.subS2pulses[n].first/e.tpc.maxS2area;
  const float yy = e.tpc.height2length[e.tpc.subS2pulses[n].second];
  return TMath::Log10(yy) < m*TMath::Log10(xx)+b;
}

// Multiple scatter cut
bool CXMSsubS2H2W(MyEvent const& e, int n) { 
  if (e.tpc.maxS1pulseID<0 || e.tpc.maxS2pulseID<0) return false; // no S1 or S2
  if (e.tpc.subS2pulses[n].second<0) return false; // sub pulse n doesn't exist - not part of the multiple scatter
  return !CXSSsubS2H2W(e,n); 
}


//________________________________________________________
Bool_t od_calib::Process(Long64_t entry)
{

  //if (entry < 50000) return kTRUE;

  GetEntry(entry);
  //  if (entry%100==0) cout << "Processing "<<entry<<endl;
  if (nentries%10==0) cout << "Processing "<<nentries<<endl;
  //cout << entry << endl;
  nentries++;
  //----------------------------------
  // Build derived RQs from input RQs
  deriveDetRQs(e, e.tpcHG, e.tpc);
  //cout << "Finished deriving det RQs" << endl;
  deriveMctRQs(e);
  //cout << "Finished deriving mct RQs" << endl;

  //----------------------------------
  // Define cut booleans
  const bool cxTPC = CXTPC(e);
  const bool cxnS1only = CXnS1only(e);
  const bool cxPPT = CXPPT(e);
  const bool cxMC4MeV = CXMC4MeV(e);
  const bool cxOtherS2notBiggest = CXOtherS2notBiggest(e);
  const bool cxHE = CXHE(e);
  const bool cxLE = CXLE(e);
  const bool cxS1first = CXS1first(e);
  const bool qual = cxPPT && cxMC4MeV && cxS1first;// && cxTPC && cxnS1only && cxOtherS2notBiggest; 
  const bool cxSSsubS2H2W = CXSSsubS2H2W(e,0);
  const bool cxMSsubS2H2W = CXMSsubS2H2W(e,0);
  const bool cxSS =  qual && cxSSsubS2H2W;
  const bool cxMS = qual && cxMSsubS2H2W;
  //cout << "Finished cuts" << endl;

  if (cxSS) buildSS(e);
  //cout << "Built SS" << endl;
  if (cxMS) buildMS(e);
  //cout << "Built MS" << endl;
  
  //----------------------------------
  // Fill histograms
 

  h2_all_s1_s2->Fill(e.tpc.maxS1area, e.tpc.maxS2area);
  h1_all_s1_s2->Fill(e.tpc.maxS1area);
  h1_all_s1_s2->Fill(e.tpc.maxS2area);



  //try stuff
  if (e.tpc.maxS2area != 0) {
    h2_all_subS2maxS2ratio_h2w->Fill(e.tpc.subS2pulses[0].first/e.tpc.maxS2area, e.tpc.height2length[e.tpc.subS2pulses[0].second]);
  }

  //other variables
  if (e.trgTime_ns<first_event_time_ns || first_event_time_ns==0 ) {
    std::cout<<"first " <<e.trgTime_ns<< " was "<<first_event_time_ns<<endl;
    first_event_time=e.trgTime_s;
    first_event_time_ns=e.trgTime_ns;
  }
  if(e.trgTime_ns>final_event_time_ns || final_event_time_ns==0){
    std::cout<<"final " <<e.trgTime_ns<< " was "<<final_event_time_ns<<endl;
    final_event_time=e.trgTime_s;
    final_event_time_ns=e.trgTime_ns;
  }
  lastentry=((int) entry)+1;


  // std::cout<<"------------------ " << entry <<" ------------------"<<std::endl;
  // std::cout<<"trig time "<<e.trgTime_s<<" LG "<<e.odLG.nPulses<<", HG: "<<e.odHG.nPulses<<std::endl;


  //Fill TPC LG pulse info
  h1_tpcLG_nPulses->Fill(e.tpcLG.nPulses);
  for (int p=0; p<e.tpcLG.nPulses; p++) {
    h1_tpcLG_pulseStartTime->Fill(e.tpcLG.pulseStartTime[p]);
    h1_tpcLG_pulseEndTime->Fill(e.tpcLG.pulseEndTime[p]);
    h1_tpcLG_pulseLength->Fill(float(e.tpcLG.pulseEndTime[p]-e.tpcLG.pulseStartTime[p])/1000);
    h1_tpcLG_pulseArea->Fill(e.tpcLG.pulseArea[p]);
    h1_tpcLG_nPulsesSat->Fill(accumulate(e.tpcLG.chSaturated[p].begin(),e.tpcLG.chSaturated[p].end(), 0));
    h1_tpcLG_chPulseAreaSize->Fill(e.tpcLG.chPulseArea[p].size());
    h2_tpcLG_pulseArea_pulseLength->Fill(e.tpcLG.pulseArea[p], float(e.tpcLG.pulseEndTime[p]-e.tpcLG.pulseStartTime[p])/1000);
    h2_tpcLG_pulseArea_peakAmp->Fill(e.tpcLG.pulseArea[p], e.tpcLG.peakAmp[p]);
    for(int c = 0; c < e.tpcLG.chPulseArea[p].size(); c++) {
      h1_tpcLG_chPulseArea->Fill(e.tpcLG.chPulseArea[p][c]);
      h1_tpcLG_chPeakTime->Fill(e.tpcLG.chPeakTime[p][c]);
      h1_tpcLG_chPeakAmp->Fill(e.tpcLG.chPeakAmp[p][c]);
    }
  }



  //Fill OD LG pulse info
  h1_odLG_nPulses->Fill(e.odLG.nPulses);
  for (int p=0; p<e.odLG.nPulses; p++) {
    h1_odLG_pulseStartTime->Fill(e.odLG.pulseStartTime[p]);
    h1_odLG_pulseEndTime->Fill(e.odLG.pulseEndTime[p]);
    h1_odLG_pulseLength->Fill(float(e.odLG.pulseEndTime[p]-e.odLG.pulseStartTime[p])/1000);
    h1_odLG_pulseArea->Fill(e.odLG.pulseArea[p]);
    h1_odLG_nPulsesSat->Fill(accumulate(e.odLG.chSaturated[p].begin(),e.odLG.chSaturated[p].end(), 0));
    h1_odLG_chPulseAreaSize->Fill(e.odLG.chPulseArea[p].size());
    h2_odLG_pulseArea_pulseLength->Fill(e.odLG.pulseArea[p], float(e.odLG.pulseEndTime[p]-e.odLG.pulseStartTime[p])/1000);
    h2_odLG_pulseArea_peakAmp->Fill(e.odLG.pulseArea[p], e.odLG.peakAmp[p]);
    for(int c = 0; c < e.odLG.chPulseArea[p].size(); c++) {
      h1_odLG_chPulseArea->Fill(e.odLG.chPulseArea[p][c]);
      h1_odLG_chPeakTime->Fill(e.odLG.chPeakTime[p][c]);
      h1_odLG_chPeakAmp->Fill(e.odLG.chPeakAmp[p][c]);
    }
  }


  //Fill HG pulse info
  h1_odHG_nPulses->Fill(e.odHG.nPulses);
  for (int p=0; p<e.odHG.nPulses; p++) {
    
    h1_odHG_pulseStartTime->Fill(e.odHG.pulseStartTime[p]);
    h1_odHG_pulseEndTime->Fill(e.odHG.pulseEndTime[p]);
    h1_odHG_pulseLength->Fill(float(e.odHG.pulseEndTime[p]-e.odHG.pulseStartTime[p])/1000);
    h1_odHG_pulseArea->Fill(e.odHG.pulseArea[p]);
    h1_odHG_nPulsesSat->Fill(accumulate(e.odHG.chSaturated[p].begin(),e.odHG.chSaturated[p].end(), 0));
    h1_odHG_chPulseAreaSize->Fill(e.odHG.chPulseArea[p].size());
    h2_odHG_pulseArea_pulseLength->Fill(e.odHG.pulseArea[p], float(e.odHG.pulseEndTime[p]-e.odHG.pulseStartTime[p])/1000);
    h2_odHG_pulseArea_peakAmp->Fill(e.odHG.pulseArea[p], e.odHG.peakAmp[p]);
    for(int c = 0; c < e.odHG.chPulseArea[p].size(); c++) {
      h1_odHG_chPulseArea->Fill(e.odHG.chPulseArea[p][c]);
      h1_odHG_chPeakTime->Fill(e.odHG.chPeakTime[p][c]);
      h1_odHG_chPeakAmp->Fill(e.odHG.chPeakAmp[p][c]);
    }
  }


  //test now for TPC pulses

  //  std::cout<<"Printing TPC LG pulses "<<std::endl;
  if(0){
  for (int p=0; p<e.tpcLG.nPulses; p++) {
    //    std::cout<<"maxS1area: "<<e.tpcLG.maxS1area<<" maxS2area: "<<e.tpcLG.maxS2area<<std::endl;;
    //    std::cout<<"pulseArea: "<<e.tpcLG.pulseArea[p]<<" probs1: "<<e.tpcLG.s1prob[p]<<" probs2 "<<e.tpcLG.s2prob[p]<<std::endl;
    if (e.tpcLG.s1prob[p]==1 || e.tpcLG.s2prob[p]==1){
      std::cout<<"pulseArea: "<<e.tpcLG.pulseArea[p]<<" probs1: "<<e.tpcLG.s1prob[p]<<" probs2 "<<e.tpcLG.s2prob[p]<<std::endl;
      std::cout<<"maxS1area: "<<e.tpcLG.maxS1area<<" maxS2area: "<<e.tpcLG.maxS2area<<std::endl;//" height2lenght"<<e.tpcLG.height2length[p]<<std::endl;

    }
    //std::cout<<p<<std::endl;
  //   std::cout<<p<<" " <<e.tpcLG.pulseStartTime[p]<<"-"<<e.tpcLG.pulseEndTime[p]<<" = "<<float(e.tpcLG.pulseEndTime[p]-e.tpcLG.pulseStartTime[p])/1000<<" ms"<<" area:" <<e.tpcLG.pulseArea[p]<<", id: "<<e.tpcLG.pulseID[p];
  //   int chsSat = accumulate(e.tpcLG.chSaturated[p].begin(),e.tpcLG.chSaturated[p].end(), 0);
  //   cout<<"# sat. ch." <<chsSat<<"# ch. area. "<<e.tpcLG.chPulseArea[p].size()<<std::endl;
  //   float sum_ch_areas=0;

  //   for(int c = 0; c < e.tpcLG.chPulseArea[p].size(); c++) {
  //     sum_ch_areas+=e.tpcLG.chPulseArea[p][c];
  //     std::cout<<"\t channel "<<c<<" area: "<<e.tpcLG.chPulseArea[p][c];
  //     std::cout<<" peak amp: "<<e.tpcLG.chPeakAmp[p][c];
  //     std::cout<<" peak time: "<<e.tpcLG.chPeakTime[p][c];
  //     std::cout<<" peak ID: " <<e.tpcLG.chID[p][c]; // TPC count is 801-920
  //     std::cout<<" ch satD: " <<e.tpcLG.chSaturated[p][c]<<std::endl; // TPC count is 801-920
  //   }
  //   std::cout<<sum_ch_areas<<std::endl;
  }
  }

  // std::cout<<"Printing LG pulses "<<std::endl;
  // for (int p=0; p<e.odLG.nPulses; p++) {
  //   //std::cout<<p<<std::endl;
  //   std::cout<<p<<" " <<e.odLG.pulseStartTime[p]<<"-"<<e.odLG.pulseEndTime[p]<<" = "<<float(e.odLG.pulseEndTime[p]-e.odLG.pulseStartTime[p])/1000<<" ms"<<" area:" <<e.odLG.pulseArea[p]<<", id: "<<e.odLG.pulseID[p];
  //   int chsSat = accumulate(e.odLG.chSaturated[p].begin(),e.odLG.chSaturated[p].end(), 0);
  //   cout<<"# sat. ch." <<chsSat<<"# ch. area. "<<e.odLG.chPulseArea[p].size()<<std::endl;
  //   float sum_ch_areas=0;

  //   for(int c = 0; c < e.odLG.chPulseArea[p].size(); c++) {
  //     sum_ch_areas+=e.odLG.chPulseArea[p][c];
  //     std::cout<<"\t channel "<<c<<" area: "<<e.odLG.chPulseArea[p][c];
  //     std::cout<<" peak amp: "<<e.odLG.chPeakAmp[p][c];
  //     std::cout<<" peak time: "<<e.odLG.chPeakTime[p][c];
  //     std::cout<<" peak ID: " <<e.odLG.chID[p][c]; // OD count is 801-920
  //     std::cout<<" ch satD: " <<e.odLG.chSaturated[p][c]<<std::endl; // OD count is 801-920
  //   }
  //   std::cout<<sum_ch_areas<<std::endl;
  // }


  // std::cout<<"Printing HG pulses "<<std::endl;
  // for (int p=0; p<e.odHG.nPulses; p++) {
  //   //std::cout<<p<<std::endl;
  //   std::cout<<p<<" start time " <<e.odHG.pulseStartTime[p]<<"-"<<e.odHG.pulseEndTime[p]<<" = "<<float(e.odHG.pulseEndTime[p]-e.odHG.pulseStartTime[p])/1000<<" ms"<<" area:" <<e.odHG.pulseArea[p]
  // 	     <<" photonCount: "<<e.odHG.photonCount[p]
  // 	     <<" id: "<<e.odHG.pulseID[p]
  // 	     <<" peak amp: " <<e.odHG.peakAmp[p] << " peakTime: "<<e.odHG.peakTime[p];
  //   int chsSat = accumulate(e.odHG.chSaturated[p].begin(),e.odHG.chSaturated[p].end(), 0);
  //   cout<<" # sat. ch." <<chsSat<<" # ch. area. "<<e.odHG.chPulseArea[p].size()<<std::endl;
  //   float sum_ch_areas=0;

  //   for(int c = 0; c < e.odHG.chPulseArea[p].size(); c++) {
  //     sum_ch_areas+=e.odHG.chPulseArea[p][c];
  //     std::cout<<"\t channel "<<c<<" area: "<<e.odHG.chPulseArea[p][c];
  //     std::cout<<" peak amp: "<<e.odHG.chPeakAmp[p][c];
  //     std::cout<<" peak time: "<<e.odHG.chPeakTime[p][c];
  //     std::cout<<" peak ID: " <<e.odHG.chID[p][c]; // OD count is 801-920
  //     std::cout<<" ch satD: " <<e.odHG.chSaturated[p][c]<<std::endl; // OD count is 801-920
  //   }
  //   std::cout<<sum_ch_areas<<std::endl;
  // }

  
  
  return kTRUE;
}

//________________________________________________________
void od_calib::SlaveTerminate()
{
  //  std::cout<<"first event "<<first_event_time<<" last " << final_event_time<<" "<<float(final_event_time_ns-first_event_time_ns)/(1000000*lastentry)<<endl;
  std::cout<<"ns: first event "<<first_event_time<<" last " << final_event_time<<" "<<float(final_event_time_ns-first_event_time_ns)/(1000000*nentries)<<endl;
  //  std::cout<<"sec: first event "<<first_event_time<<" last " << final_event_time<<" "<<float(final_event_time-first_event_time)/(lastentry)<<". rate: "<<lastentry/(float(final_event_time-first_event_time)) <<std::endl;
  std::cout<<"rate: " <<(1000000*nentries)/float(final_event_time_ns-first_event_time_ns)<<std::endl;
  std::cout<<nentries<<std::endl;
  h1_rate->SetBinContent(3,float(final_event_time_ns-first_event_time_ns)/(1000000*lastentry));

  // int nEvents = events->GetEntries();
  
  // events->GetEntry(0);
  // ulong start = triggerTimestamp_s*1E9 + triggerTimestamp_ns;
  // events->GetEntry(nEvents-1);
  // ulong end = triggerTimestamp_s*1E9 + triggerTimestamp_ns;

  // livetime_ns = end - start;
  // livetime_s = (end - start)/1E9;
  

}

//________________________________________________________
void od_calib::Terminate()
{
  
  TFile* outfile = TFile::Open(fOutFileName,"recreate");



  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_all_s1_s2")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_all_s1_s2")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_rate")))->Write();


  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_nPulses")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_pulseStartTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_pulseEndTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_pulseLength")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_pulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_photonCount")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_peakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_peakAmp")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_nPulsesSat")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_chPulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_chPulseAreaSize")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_chPeakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_tpcLG_chPeakAmp")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_tpcLG_pulseArea_pulseLength")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_tpcLG_pulseArea_peakAmp")))->Write();

  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_nPulses")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_pulseStartTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_pulseEndTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_pulseLength")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_pulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_photonCount")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_peakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_peakAmp")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_nPulsesSat")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_chPulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_chPulseAreaSize")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_chPeakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odLG_chPeakAmp")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_odLG_pulseArea_pulseLength")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_odLG_pulseArea_peakAmp")))->Write();


  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_nPulses")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_pulseStartTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_pulseEndTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_pulseLength")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_pulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_photonCount")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_peakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_peakAmp")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_nPulsesSat")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_chPulseArea")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_chPulseAreaSize")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_chPeakTime")))->Write();
  (dynamic_cast<TH1F*>(fOutput->FindObject("h1_odHG_chPeakAmp")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_odHG_pulseArea_pulseLength")))->Write();
  (dynamic_cast<TH2F*>(fOutput->FindObject("h2_odHG_pulseArea_peakAmp")))->Write();


  outfile->Close(); 
    
}

//____________________________________
// Make a decision on whether to use HG or LG based on whether or not the pulse is saturated
pair <MyDetectorPulses, int> det(MyEvent & e, int p) {
  int chsSat = accumulate(e.tpcHG.chSaturated[p].begin(),e.tpcHG.chSaturated[p].end(), 0);
  //cout << "chsSat: " << chsSat << endl;
  if (chsSat == 0) return make_pair(e.tpcHG,p);
  //cout << "Not crazy saturated" << endl;
  if (p > 0) {
    if (chsSat == 1 & e.tpc.pulseClass[p-1]==PulseClass::S2 & e.tpcHG.saturated[p-1]) return make_pair(e.tpcHG,p);
  }
  //cout << "Not sorta saturated - use lg then" << endl;
  //cout << "p " << p << " LG " << e.tpcHG.HGLGpulseID[p] << endl;
  if (e.tpcHG.HGLGpulseID[p] == -1) return make_pair(e.tpcHG,p);
  return make_pair(e.tpcLG,e.tpcHG.HGLGpulseID[p]);
}

//________________________________________________________
// Build derived RQs for detector
void deriveDetRQs(MyEvent & e, MyDetectorPulses & defdet, MyDetectorPulses & newdet) {
  
  //cout << "New event" << endl;

  newdet.pulseClass.resize(defdet.nPulses);
  //cout << "Resized" << endl;
  for (int p=0; p<defdet.nPulses; ++p) {
    if      (defdet.s1prob[p])  newdet.pulseClass[p] = PulseClass::S1;
    else if (defdet.s2prob[p])  newdet.pulseClass[p] = PulseClass::S2;
    else if (defdet.speProb[p]) newdet.pulseClass[p] = PulseClass::SPE;
    else if (defdet.mpeProb[p]) newdet.pulseClass[p] = PulseClass::MPE;
    else if (defdet.seProb[p])  newdet.pulseClass[p] = PulseClass::SE;
    else if (defdet.otherProb[p]) newdet.pulseClass[p] = PulseClass::OTHER;
    else if (defdet.otherS2prob[p]) newdet.pulseClass[p] = PulseClass::OTHERS2;
    else newdet.pulseClass[p] = PulseClass::ALL;

    //cout << "Assigned probabilities for pulse" << endl;
  }

  //cout << "Assigned probabilities for all pulses" << endl;

  newdet.maxPulseID = -1;
  newdet.maxPulseArea = 0;
  newdet.maxPulseClass = PulseClass::ALL;
  newdet.maxS1pulseID = -1;
  newdet.maxS1area = 0;
  newdet.maxS2pulseID = -1;
  newdet.maxS2area = 0;
  newdet.subS2pulses.clear();
  newdet.subS2pulses.push_back(make_pair(float(0),-1));
  for (int p=0; p<defdet.nPulses; ++p) {
    if (defdet.pulseArea[p] > newdet.maxPulseArea) {
      //cout << "Found potential max pulse " << p << endl;
      newdet.maxPulseID = p;
      newdet.maxPulseArea = defdet.pulseArea[p];
      newdet.maxPulseClass = newdet.pulseClass[p];
    }
    if (defdet.s1prob[p] == 1 && defdet.pulseArea[p] > newdet.maxS1area) {
      newdet.maxS1pulseID = defdet.pulseID[p];
      newdet.maxS1area = defdet.pulseArea[p];
      //cout << "Found potential max S1" << endl;
      //cout << "Area: " << newdet.maxS1area << " " << p << endl;
    }
    if (defdet.s2prob[p] == 1 && defdet.pulseArea[p] > newdet.maxS2area) {
      newdet.maxS2pulseID = defdet.pulseID[p];
      newdet.maxS2area = defdet.pulseArea[p];
      //cout << "Found potential max S2" << endl;
      //cout << "Area: " << newdet.maxS2area << " " << p << endl;
    }
  }

  //cout << "Found max pulses" << endl;

  if (newdet.maxPulseID > -1) newdet.maxPulseArea = det(e,newdet.maxPulseID).first.pulseArea[det(e,newdet.maxPulseID).second];
  //cout << "Did max pulse" << endl;
  if (newdet.maxS1pulseID > -1) {
    newdet.maxS1area = det(e,newdet.maxS1pulseID).first.pulseArea[det(e,newdet.maxS1pulseID).second];
    //cout << "Max S1 area = " << newdet.maxS1area << endl;
  }
  //cout << "Did S1" << endl;
  //cout << newdet.maxS2pulseID << endl;
  if (newdet.maxS2pulseID > -1) {
    newdet.maxS2area = det(e,newdet.maxS2pulseID).first.pulseArea[det(e,newdet.maxS2pulseID).second];
    //cout << "Max S2 area = " << newdet.maxS2area << endl;
  }

  //cout << "Assigned max pulses" << endl;
  for (int p=0; p<defdet.nPulses; ++p) {
    pair < MyDetectorPulses, int > decdet = det(e,p);
    if (defdet.s2prob[p] == 1 && defdet.pulseID[p] > newdet.maxS1pulseID && decdet.first.pulseArea[decdet.second] < newdet.maxS2area) {
      if (newdet.subS2pulses.front() == make_pair(float(0),-1)) newdet.subS2pulses.erase(newdet.subS2pulses.begin());
      newdet.subS2pulses.push_back(make_pair(decdet.first.pulseArea[decdet.second],defdet.pulseID[p]));
      //cout << "Found a sub S2" << endl;
      //cout << "Pushed back: " << det.pulseArea[p] << " " << det.pulseID[p] << endl;
    }
  }
  //cout << "Found list" << endl;
  sort(newdet.subS2pulses.begin(), newdet.subS2pulses.end(), sortinrev);
  //if (newdet.subS2pulses[0].second != -1) newdet.subS2pulses[0] = *max_element(newdet.subS2pulses.begin(), newdet.subS2pulses.end(), comp_pair);
  
  //cout << "Done and sorted" << endl;
  //cout << "Max S2: " << newdet.maxS2area << " " << newdet.maxS2pulseID << endl;
  //cout << "Listing sub S2s: " << endl;
  /*for (int i=0; i<newdet.subS2pulses.size(); i++)
    {
        // "first" and "second" are used to access
        // 1st and 2nd element of pair respectively
        cout << newdet.subS2pulses[i].first << " "
             << newdet.subS2pulses[i].second << endl;
	     }*/

  newdet.chsSat.resize(defdet.nPulses);
  newdet.t0595.resize(defdet.nPulses);
  newdet.promptFrac200ns.resize(defdet.nPulses);
  newdet.height2length.resize(defdet.nPulses);
  //cout << "Resized again" << endl;
  for (int p=0; p<defdet.nPulses; ++p) {
    pair < MyDetectorPulses, int > decdet = det(e,p);
    //cout << "Found det for pulse " << p+1 << " of " << defdet.nPulses << endl;
    newdet.chsSat[p] = accumulate(e.tpcHG.chSaturated[p].begin(),e.tpcHG.chSaturated[p].end(), 0);
    //if (newdet.chsSat[p] > 0 )cout << newdet.chsSat[p] << " in pulse " << p << endl;
    newdet.t0595[p] = decdet.first.aft95[decdet.second] - decdet.first.aft5[decdet.second];
    newdet.promptFrac200ns[p] = decdet.first.pulseArea200ns[decdet.second] / decdet.first.pulseArea[decdet.second];
    newdet.height2length[p] = decdet.first.peakAmp[decdet.second] / (decdet.first.aft95[decdet.second] - decdet.first.aft5[decdet.second]);
  } 
}

//________________________________________________________
// Build derived RQs for MCT
void deriveMctRQs(MyEvent & e) {  
  e.mcPulseLength.resize(e.mcPulseFirstPheTime_ns.size());
  for (size_t p=0; p<e.mcPulseFirstPheTime_ns.size(); ++p) {
    e.mcPulseLength[p] = e.mcPulseLastPheTime_ns[p] - e.mcPulseFirstPheTime_ns[p];
  }
}

//________________________________________________________
// Build single scatters and corrections
// Assumes maxS1pulseID and maxS2pulseID are the S1 and S2
float correctS1(float pulseAreaTop, float pulseAreaBot, float driftTime) {
  double toppars[6]={1.35352e+00,2.53625e-06,-1.99506e-11,4.52959e-17,-4.72829e-23,1.85961e-29};
  double botpars[6]={4.88547e-01,1.48639e-06,-2.74680e-12,7.58603e-18,-8.49328e-24,3.71999e-30};
  const float s1top = pulseAreaTop;
  const float s1bot = pulseAreaBot;
  double s1topCorrFactor = 0;
  double s1botCorrFactor = 0;
  for(int i=0; i<6; i++) {
    s1topCorrFactor+=toppars[i]*TMath::Power(driftTime,i);
    s1botCorrFactor+=botpars[i]*TMath::Power(driftTime,i);
  }
  return s1top/s1topCorrFactor + s1bot/s1botCorrFactor;
}

float correctS2(time_t trgTime_s, float s2area, float driftTime) {
  
  float lifetimes[32] = {860., 870., 880.,  890., 905., 905., 920., 915., 930., 925.,
                         945., 940., 950., 1000.,1000.,1000.,1000.,1000.,1000.,1000.,
                         1000.,1000.,1000.,1000.,1000.,1000.,1000.,1000.,1000.,1000.,1000.,1000.}; //[us]

  time_t stamp = trgTime_s;
  tm* timePtr = gmtime(&stamp);
  int day = timePtr->tm_mday;
  int month = timePtr->tm_mon;

  int lifetime = 1000.;
  if (month==3)
    lifetime = lifetimes[day-1];
  return s2area*TMath::Exp(driftTime/1000./lifetime);
}

float correctRpos(float rPos, float driftTime) {

  double r2pars[6]={5671.1,-0.0175672,7.86025e-08,-1.77335e-13,1.9308e-19,-8.16797e-26};
  double r2_0 =0;
  for(int i=0; i<6; i++)
    r2_0+=r2pars[i]*TMath::Power(driftTime,i);
    
  double rPosCorrFactor = rPos*rPos/r2_0;
  return TMath::Sqrt(kTPCradius*kTPCradius*rPosCorrFactor);
}

void buildSS(MyEvent & e) {
  e.ss.s1pulseID = e.tpc.maxS1pulseID;
  e.ss.s2pulseID = e.tpc.maxS2pulseID;
  pair < MyDetectorPulses, int > detS1 = det(e,e.ss.s1pulseID);
  pair < MyDetectorPulses, int > detS2 = det(e,e.ss.s2pulseID);
  e.ss.s1area = detS1.first.pulseArea[detS1.second];
  e.ss.s2area = detS2.first.pulseArea[detS2.second];
  e.ss.driftTime = ((detS2.first.pulseStartTime[detS2.second] + detS2.first.aft5[detS2.second]) -
                    (detS1.first.pulseStartTime[detS1.second] + detS1.first.aft5[detS1.second]));
  e.ss.corrS1area = correctS1(detS1.first.pulseAreaTop[detS1.second], detS1.first.pulseAreaBot[detS1.second], e.ss.driftTime);
  e.ss.corrS2area = correctS2(e.trgTime_s, e.ss.s2area, e.ss.driftTime);
  e.ss.xPos = detS2.first.xPos[detS2.second];
  e.ss.yPos = detS2.first.yPos[detS2.second];
  e.ss.zPos = kTPCheight*(1.-e.ss.driftTime/1e3/880.);
  e.ss.rPos = TMath::Sqrt(e.ss.xPos*e.ss.xPos + e.ss.yPos*e.ss.yPos);
  e.ss.corrRpos = correctRpos(e.ss.rPos, e.ss.driftTime);
}

void buildMS(MyEvent & e) {
  //cout << "Building MS" << endl;
  e.ms.s1pulseID = e.tpc.maxS1pulseID;
  pair < MyDetectorPulses, int > detS1 = det(e,e.ms.s1pulseID);
  e.ms.s1area = detS1.first.pulseArea[detS1.second];

  //cout << "S1: " << e.ms.s1pulseID << " " << e.ms.s1area << endl;

  e.ms.s2pulseIDs.clear();
  e.ms.s2areas.clear();
  e.ms.driftTimes.clear();
  e.ms.corrS2areas.clear();
  e.ms.xPos.clear();
  e.ms.yPos.clear();
  e.ms.zPos.clear();
  e.ms.rPos.clear();
  e.ms.corrRpos.clear();

  e.ms.s2pulseIDs.push_back(e.tpc.maxS2pulseID);
  e.ms.s2areas.push_back(e.tpc.maxS2area);

  //cout << "S2: " << e.ms.s2pulseIDs[0] << " " << e.ms.s2areas[0] << endl;

  for (size_t p=0; p<e.tpc.subS2pulses.size(); ++p) {
    //cout << "In for loop" << endl;
    if (CXMSsubS2H2W(e,p)) {
      //cout << "In if" << endl;
      e.ms.s2pulseIDs.push_back(e.tpc.subS2pulses[p].second);
      //cout << "Pushed pulse ID" << endl;
      e.ms.s2areas.push_back(e.tpc.subS2pulses[p].first);
      //cout << "S2: " << e.tpc.subS2pulses[p].second << " " << e.tpc.subS2pulses[p].first << endl;
    }
  }

  //cout << "Did sub S2s" << endl;

  e.ms.wDriftTime = 0;
  float totS2area = 0;
  for (size_t p=0; p<e.ms.s2areas.size(); ++p) {
    pair < MyDetectorPulses, int > detS2 = det(e,e.ms.s2pulseIDs[p]);
    e.ms.driftTimes.push_back((detS2.first.pulseStartTime[detS2.second] + detS2.first.aft5[detS2.second]) -
			       (detS1.first.pulseStartTime[detS1.second] + detS1.first.aft5[detS1.second]));
    e.ms.wDriftTime += e.ms.driftTimes[p] * e.ms.s2areas[p];
    totS2area += e.ms.s2areas[p];
    e.ms.corrS2areas.push_back(correctS2(e.trgTime_s, e.ms.s2areas[p], e.ms.driftTimes[p]));
    e.ms.xPos.push_back(detS2.first.xPos[detS2.second]);
    e.ms.yPos.push_back(detS2.first.yPos[detS2.second]);
    e.ms.zPos.push_back(kTPCheight*(1.-e.ms.driftTimes[p]/1e3/880.));
    e.ms.rPos.push_back(TMath::Sqrt(e.ms.xPos[p]*e.ms.xPos[p] + e.ms.yPos[p]*e.ms.yPos[p]));
    e.ms.corrRpos.push_back(correctRpos(e.ms.rPos[p], e.ms.driftTimes[p]));
  }
  e.ms.wDriftTime = e.ms.wDriftTime / totS2area;

  //cout << "Did drift time, corrections, and positions" << endl;

  e.ms.corrS1area = correctS1(detS1.first.pulseAreaTop[detS1.second], detS1.first.pulseAreaBot[detS1.second], e.ms.wDriftTime);

}

//________________________________________________________
bool sortinrev(const pair<float,int> &a, 
               const pair<float,int> &b)
{
       return (a.first > b.first);
}

bool comp_pair(const pair<float,int> &a, 
               const pair<float,int> &b)
{
       return (a.first < b.first);
}




