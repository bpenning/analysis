/*
  usage :  This code serve the purpose to create a ntuple for G4_103_MCTruth_Validation_error.cc to read.
  User needs to compile the code first : g++ -I /usr/common/usg/software/ROOT/5.34.20/include/ `root-config --cflags --libs` -o MCTruth_ntuple_create MCTruth_ntuple_create.cc /cvmfs/lz.opensciencegrid.org/BACCARAT/release-2.4.0/lib/libBaccMCTruthEvent.so
  Remeber to put the path of your files in line 86.
*/

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <vector>
#include <functional>
#include <numeric>
#include <cmath>
#include <dirent.h>
#include <sstream>

#include <sys/stat.h>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TH1F.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLine.h"
#include "TObjString.h"
#include "TKey.h"
#include "TCollection.h"
#include "TNtuple.h"

#include "/cvmfs/lz.opensciencegrid.org/BACCARAT/release-2.4.0/tools/BaccMCTruthEvent.hh"
using namespace std;
vector<string> filenamev;
void NormalizedSum(TH1F* hsum){

  double norm = hsum->Integral(1,hsum->GetNbinsX(),"width");
  hsum->Scale(1 / norm , "width");

}
bool has_suffix(const string& s, const string& suffix)
{
  return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}
//TChain* ChainFiles(const char *name, TChain* tree){
void ChainFiles(const char *name){
  //filename.clear();
  char buff[100];
  //string buff;
  DIR *dir = opendir(name);
  cout<<" The dir is "<<name<<endl;
  if(!dir)
    {
      cout << "Error() opening " << name << endl;
    }
  dirent *entry;
  while((entry = readdir(dir))!=NULL)
    {
      if(has_suffix(entry->d_name, ".root"))
        {
	  cout<<" File name is : "<<entry->d_name<<endl;

	  strcpy(buff,entry->d_name);
	  stringstream ss;
	  //stringstream ss;
	  ss << buff;
	  filenamev.push_back(ss.str());

        }
    }
  //return tree;
  closedir(dir);
}
int main(int argc, char *arg[]){
  //int version = atoi(arg[1]);
  string ifilename = arg[1];
  string ofilename = arg[2];

  std::cout<<"in "<<ifilename<<endl;
  std::cout<<"out "<<ofilename<<endl;
  TChain* tree = new TChain("MCTruthTree","MCTruthTree");
  char sub[500];
  // Put path to the files here
  //    sprintf(sub,"/global/homes/w/wangryan/lz_project_data/wangbtc/G4_Validation_data/MCTruth_%d_optical/",version);
  //sprintf(sub,"/global/homes/p/penning/sim/AmLi_mdc3_v8",version);



  // ChainFiles(sub);
  // for (int i=0;i<filenamev.size();i++){
  //     cout<<" In main loop, File name is : "<<filenamev[i]<<endl;
  // 	//        TString path = sub + filenamev[i];
  //     TString path = filenamev[i];
  //     tree->Add(path.Data());
  // }
  // cout<<" Test if it loop over dir "<<endl;


  BaccMCTruthEvent* event =0;
  TFile *inroot = new TFile(ifilename.c_str(),"OPEN");
  tree->Add(ifilename.c_str());

  TFile *outroot = new TFile(ofilename.c_str(),"RECREATE");
  TNtuple* ntPhoton = new TNtuple("ntPhoton","ntPhoton","time:wavelenth:pmt:inter:vertex");
  TNtuple* ntParent = new TNtuple("ntParent","ntParent","energy:time:evnumber:evFirstTime:evLastTime:energy");
  TNtuple* ntVertex = new TNtuple("ntVertex","ntVertex","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexhigh = new TNtuple("ntVertexhigh","ntVertexhigh","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexlow = new TNtuple("ntVertexlow","ntVertexlow","x:y:z:energy:time:s1:s2");

  TNtuple* ntVertexLXe = new TNtuple("ntVertexLXe","ntVertexLXe","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexLXeSkin = new TNtuple("ntVertexLXeSkin","ntVertexLXeSkin","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexLXeSkinB = new TNtuple("ntVertexLXeSkinB","ntVertexLXeSkinB","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexReverse = new TNtuple("ntVertexReverse","ntVertexReverse","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexInnerGas = new TNtuple("ntVertexInnerGas","ntVertexInnerGas","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexOD = new TNtuple("ntVertexOD","ntVertexOD","x:y:z:energy:time:s1:s2");

  TNtuple* ntVertexBSB = new TNtuple("ntVertexBSB","ntVertexBSB","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexTSB = new TNtuple("ntVertexTSB","ntVertexTSB","x:y:z:energy:time:s1:s2");

  TNtuple* ntVertexNSB = new TNtuple("ntVertexNSB","ntVertexNSB","x:y:z:energy:time:s1:s2");

  TNtuple* ntVertexmixed = new TNtuple("ntVertexmixed","ntVertexmixed","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexe = new TNtuple("ntVertexe","ntVertexe","x:y:z:energy:time:s1:s2");
  TNtuple* ntVertexgamma = new TNtuple("ntVertexgamma","ntVertexgamma","x:y:z:energy:time:s1:s2");

  tree->SetBranchAddress("MCTruthEvent",&event);

  TH1F* total_event_energy = new TH1F("total_event_energy","total_event_energy",16000,0,16000);
  TH1F* total_photon = new TH1F("total_photon","total_photon",16000,0,16000);
  TH2F* photon_energy = new TH2F("photon_energy","photon_energy",16000,0,16000,16000,0,16000);
  string VolumeName[8]={"LiquidSkinXenon","LiquidXenonTarget","LiquidSkinXenonBank","ReverseFieldRegion","InnerGaseousXenon","ScintillatorCenter"};
  string ParticleName[3]={"mixed","e-","gamma"};
  int num = tree->GetEntries();

  for(int i=0; i< num; i++){
    tree->GetEntry(i);
    double event_energy=0;

    double prompt_phe=0;
    double late_phe=0;
    double total_phe=0;
    if (i%100==0)
    cout<<" Processing "<<i<<" events "<<endl;


    int nvertex = event->vertices.size();
    //for (int j=0;j<nvertex;j++){
    for (int j=0;j<nvertex;j++){
      event_energy += event->vertices[j].fEnergyDep_keV;
      //      cout<<" Processing "<<j<<" blub "<<event->vertices[j+1].sVolumeName<<endl; //without optical this stuff doesn't seem to work
      
      // if (VolumeName[0].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexLXeSkin->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[1].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexLXe->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[2].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexLXeSkinB->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[3].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexReverse->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[4].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexInnerGas->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[5].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexOD->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[6].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexBSB->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[7].compare(event->vertices[j].sVolumeName)==0)
      // 	ntVertexTSB->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (ParticleName[0].compare(event->vertices[j].sParticleName)==0)
      // 	ntVertexmixed->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (ParticleName[1].compare(event->vertices[j].sParticleName)==0)
      // 	ntVertexe->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (ParticleName[2].compare(event->vertices[j].sParticleName)==0)
      // 	ntVertexgamma->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);


      ntVertex->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
      // if (VolumeName[6].compare(event->vertices[j].sVolumeName)==0)
      //  	continue;
      //  if (VolumeName[7].compare(event->vertices[j].sVolumeName)==0)
      //  	continue;

      ntVertexNSB->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
    }//loop over nvertex
      
    if (event_energy>100){
      for (int k=0;k<nvertex;k++){
	for (int j=0;j<nvertex;j++){
	  ntVertexhigh->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
	}
      }
    }
    if (event_energy<100){
      for (int k=0;k<nvertex;k++){
	for (int j=0;j<nvertex;j++){
	  ntVertexlow->Fill(event->vertices[j].fPosition_mm.X(),event->vertices[j].fPosition_mm.Y(),event->vertices[j].fPosition_mm.Z(),event->vertices[j].fEnergyDep_keV,event->vertices[j].fTime_ns,event->vertices[j].iS1Quanta,event->vertices[j].iS2Quanta);
	}
      }
    }
    ntParent->Fill(event->fParentEnergy_keV,event->fParentTime_ns,event->iEventNumber,event->fEventFirstPhotonTime_ns,event->fEventLastPhotonTime_ns,event_energy);

    total_event_energy->Fill(event_energy);
    total_photon->Fill(total_phe);
    photon_energy->Fill(total_phe,event_energy);
  }

  outroot->Write();
  outroot->Close();
  return 0;
}
