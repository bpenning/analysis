#ifndef MYEVENT_H
#define MYEVENT_H

#include <vector>
#include <string>

enum PulseClass : unsigned int;

class MyDetectorPulses {
public:
  MyDetectorPulses(){}
  virtual ~MyDetectorPulses(){}

  int           nPulses;
  std::vector<int>   pulseID;
  std::vector<int>   pulseStartTime;
  std::vector<int>   pulseEndTime;
  std::vector<float> pulseArea;
  std::vector<float> pulseAreaTop;
  std::vector<float> pulseAreaBot;
  std::vector<float> peakAmp;
  std::vector<int>   peakTime;
  std::vector<int>   aft1;
  std::vector<int>   aft5;
  std::vector<int>   aft25;
  std::vector<int>   aft50;
  std::vector<int>   aft75;
  std::vector<int>   aft95;
  std::vector<int>   aft99;
  std::vector<float> tba;
  std::vector<float> promptFrac50ns;
  std::vector<float> pulseArea200ns;
  std::vector<int>   rmsWidth;
  std::vector<int>   coincidence;
  std::vector<float> s1prob;
  std::vector<float> s2prob;
  std::vector<float> speProb;
  std::vector<float> mpeProb;
  std::vector<float> seProb;
  std::vector<float> otherProb;
  std::vector<float> otherS2prob;
  std::vector<PulseClass> pulseClass;
  std::vector<bool>  saturated;
  std::vector<float> xPos;
  std::vector<float> yPos;
  std::vector<int>   HGLGpulseID;
  std::vector<vector<bool>> chSaturated;
  std::vector<vector<float>> chPulseArea;
  std::vector<vector<float>> chPeakAmp;
  std::vector<vector<int>> chPeakTime;
  std::vector<vector<int>> chID;
  std::vector<int>   photonCount;

  // Derived RQs
  std::vector<int> chsSat;
  std::vector<int> t0595;
  std::vector<float> promptFrac200ns;
  std::vector<float> height2length;

  int maxPulseID; // pulseID of largest pulse
  float maxPulseArea; // area of largest pulse
  PulseClass maxPulseClass; // classification of largest pulse
  
  int   maxS1pulseID; // pulse ID of largest S1
  float maxS1area;    // area of largest S1
  int   maxS2pulseID; // pulse ID of largest S2
  float maxS2area;    // area of largest S2
  vector< pair <float, int> >   subS2pulses;// pulse area, ID of sub S2s
  
};

class MySingleScatter{
public:
  MySingleScatter(){}
  virtual ~MySingleScatter(){}

  int s1pulseID;
  int s2pulseID;
  float s1area;
  float s2area;
  float driftTime;
  float corrS1area;
  float corrS2area;
  float xPos;
  float yPos;
  float zPos;
  float rPos;
  float corrRpos;
};

class MyMultipleScatter{
public:
  MyMultipleScatter(){}
  virtual ~MyMultipleScatter(){}

  int s1pulseID;
  float s1area;
  float corrS1area;
  vector<int> s2pulseIDs;
  vector<float> s2areas;
  vector<float> corrS2areas;
  vector<float> driftTimes;
  float wDriftTime;
  vector<float> xPos;
  vector<float> yPos;
  vector<float> zPos;
  vector<float> rPos;
  vector<float> corrRpos;
};

class MyEvent {
public:
  MyEvent(){}
  virtual ~MyEvent(){}
  //TChain*  chain;

  //------------------------------------------------
  // Variable for extraction
  //------------------------------------------------
  unsigned int   eventID;
  unsigned int   trgTime_s;
  unsigned int   trgTime_ns;
  std::string    rawFileName;

  MyDetectorPulses tpcHG;
  MyDetectorPulses tpcLG;
  MyDetectorPulses tpc;
  MyDetectorPulses odHG;
  MyDetectorPulses odLG;
  MySingleScatter ss;
  MyMultipleScatter ms;
  
  // MC truth variables
  std::string parentParticle;
  
  std::vector<double> mcPulseFirstPheTime_ns;
  std::vector<double> mcPulseLastPheTime_ns;
  std::vector<double> mcPulseLength;
  std::vector<unsigned int> mcPulsePheCount;
  std::vector<string> mcVtxVolumeName;
  std::vector<double> mcVtxEnergyDep;
  std::vector<unsigned int> mcVtxRawS2photons;
  std::vector<std::vector<int> > mcArtifactPulseIndices;
  
};


#endif //MYEVENT_H
