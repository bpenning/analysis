from Gaudi.Configuration import *
import os
import DataCatalog
import dateutil.parser

try:
    inputFiles = []
    inputFiles.extend(DataCatalog.select('source','DER').select('begin_time',slice(dateutil.parser.parse(os.environ['LZAP_START_TIME']),dateutil.parser.parse(os.environ['LZAP_END_TIME']))))
except KeyError:
    try:
        inputFiles = os.environ['LZAP_INPUT_FILES'].split(',')
    except KeyError:
        inputFiles = ['/projecta/projectdirs/lz/data/MDC2/calibration/BACCARAT-3.14.1_DER-7.4.4/Na22_700/DER/lz_201704010000_000010_001000_raw.root']

try:
    outputFile = os.environ['LZAP_OUTPUT_FILE']
except KeyError:
    outputFile = 'lzap.root'

app=ApplicationMgr()
app.EvtMax  = -1
app.HistogramPersistency = "NONE"
app.EvtSel = "Ldrf"
app.OutputLevel = 3

#MessageSvc().OutputLevel = DEBUG

#################################################
# Open Ldrf (DER) file(s)
from DaqLdrfModules import DaqLdrfModulesConf
selector=DaqLdrfModulesConf.DaqLdrf__LdrfSelector("EventSelector")
selector.InputFiles = inputFiles 
app.SvcMapping.append('DaqLdrf::LdrfSelector/EventSelector')
app.SvcMapping.append('DaqLdrf::LdrfConversionSvc/LdrfConversionSvc')

app.SvcMapping.append('LZap::LzConditionsSvc/LzConditionsSvc')
app.SvcMapping.append('RQ::RQConversionSvc/RQConversionSvc')
app.SvcMapping.append('LZap::LzEvtContextSvc/LzEvtContextSvc')

from GaudiCommonSvc import GaudiCommonSvcConf
ipersist=GaudiCommonSvcConf.PersistencySvc('EventPersistencySvc')
ipersist.CnvServices=['LdrfConversionSvc']

persist=GaudiCommonSvcConf.PersistencySvc('PersistencySvc')
persist.CnvServices=['RQConversionSvc']

from RQModules import RQModulesConf
outStream=RQModulesConf.RQ__RQOutputStream('RQWriter')
outStream.OutputFile = outputFile
outStream.OptItemList = [ '/Event/Calibrated',
                          '/Event/TpcHighGain/Physics/Pulses/Boundaries',
                          '/Event/TpcHighGain/Physics/Pulses/Parameters',
                          '/Event/TpcHighGain/Physics/Pulses/ChannelPulseParameters',
                          '/Event/TpcHighGain/Physics/Pulses/Classifications',
                          '/Event/TpcHighGain/Physics/Pulses/ChannelPulsePhotons',
                          '/Event/TpcHighGain/Physics/Pulses/Photons',
                          '/Event/TpcHighGain/Physics/Pulses/S2XYPositionMercury',
                          '/Event/TpcLowGain/Physics/Pulses/Boundaries',
                          '/Event/TpcLowGain/Physics/Pulses/Parameters',
                          '/Event/TpcLowGain/Physics/Pulses/ChannelPulseParameters',
                          '/Event/TpcLowGain/Physics/Pulses/Classifications',
                          '/Event/TpcLowGain/Physics/Pulses/ChannelPulsePhotons',
                          '/Event/TpcLowGain/Physics/Pulses/Photons',
                          '/Event/TpcLowGain/Physics/Pulses/S2XYPositionMercury',
                          '/Event/Physics/Pulses/HGLGMapping',
                          '/Event/Skin/Physics/Pulses/Boundaries',
                          '/Event/Skin/Physics/Pulses/Parameters',
                          '/Event/Skin/Physics/Pulses/ChannelPulseParameters',
                          '/Event/Skin/Physics/Pulses/Classifications',
                          '/Event/Skin/Physics/Pulses/ChannelPulsePhotons',
                          '/Event/Skin/Physics/Pulses/Photons',
                          '/Event/OuterHighGain/Physics/Pulses/Boundaries',
                          '/Event/OuterHighGain/Physics/Pulses/ODParameters',
                          '/Event/OuterHighGain/Physics/Pulses/ODChannelPulseParameters',
                          '/Event/OuterLowGain/Physics/Pulses/Boundaries',
                          '/Event/OuterLowGain/Physics/Pulses/ODParameters',
                          '/Event/OuterLowGain/Physics/Pulses/ODChannelPulseParameters',
                          '/Event/Physics/PulseAreaCorrectedInteractions',
                          '/Event/Physics/PulseAreaCorrectedMultipleInteractions',
                          '/Event/Physics/Kr83mLikeInteractions',
                          '/Event/Physics/PileUpInteractions',
                          '/Event/Physics/OtherInteractions',
                          '/Event/DetectorMC' ]
outStream.Output="SVC='RQConversionSvc' OPT='RECREATE'"
outStream.EvtConversionSvc="PersistencySvc"
app.OutStream = [ outStream ]



# Configure LZap modules

from PhotonDetection import PhotonDetectionConf
from InteractionDetection import InteractionDetectionConf
from LZapServices import LZapServicesConf 
conditionsSvc=LZapServicesConf.LZap__LzConditionsSvc("LzConditionsSvc")
conditionsSvc.PMTpositionsFile="/global/project/projectdirs/lz/users/sally/UCSB/LZap_OD/PMTpositions-LZ-07Feb19.txt" 

################################################
# Common modules for all detectors

# RawEventCorrector
rawEventCorrector = PhotonDetectionConf.LZap__RawEventCorrector()

# Pod Calibrator
podCalibrator = PhotonDetectionConf.LZap__PodCalibrator('PodCalibrator')
podCalibrator.rawEventPath = "/Event/MergedRaw"
podCalibrator.preBaselineSamples = 10
podCalibrator.postBaselineSamples = 10


################################################
# TPC High Gain modules
TPCHG = 'TPC High Gain'

TPCHGPodSummer = PhotonDetectionConf.LZap__PodSummer('TPCHGPodSummer')
TPCHGPodSummer.detector = TPCHG
TPCHGPodSummer.minSeparationNs = 3000

TPCHGRollingThunder = PhotonDetectionConf.LZap__RollingThunder('TPCHGRollingThunder')
TPCHGRollingThunder.detector = TPCHG

TPCHGChannelPulseParameterizer = PhotonDetectionConf.LZap__ChannelPulseParameterizer('TPCHGChannelPulseParameterizer')
TPCHGChannelPulseParameterizer.detector = TPCHG

TPCHGPulseParameterizer = PhotonDetectionConf.LZap__PulseParameterizer('TPCHGPulseParameterizer')
TPCHGPulseParameterizer.detector = TPCHG

TPCHGPulseClassifier = PhotonDetectionConf.LZap__PulseClassifierHADES('TPCHGPulseClassifier')
TPCHGPulseClassifier.detector = TPCHG

TPCHGPhotonCounter = PhotonDetectionConf.LZap__PhotonCounter('TPCHGPhotonCounter')
TPCHGPhotonCounter.detector = TPCHG
TPCHGPhotonCounter.constThreshold_phd = 0.0015

################################################
# TPC Low Gain modules
TPCLG = 'TPC Low Gain'

TPCLGPodSummer = PhotonDetectionConf.LZap__PodSummer('TPCLGPodSummer')
TPCLGPodSummer.detector = TPCLG
TPCLGPodSummer.minSeparationNs = 3000

TPCLGRollingThunder = PhotonDetectionConf.LZap__RollingThunder('TPCLGRollingThunder')
TPCLGRollingThunder.detector = TPCLG

TPCLGChannelPulseParameterizer = PhotonDetectionConf.LZap__ChannelPulseParameterizer('TPCLGChannelPulseParameterizer')
TPCLGChannelPulseParameterizer.detector = TPCLG

TPCLGPulseParameterizer = PhotonDetectionConf.LZap__PulseParameterizer('TPCLGPulseParameterizer')
TPCLGPulseParameterizer.detector = TPCLG

TPCLGPulseClassifier = PhotonDetectionConf.LZap__PulseClassifierHADES('TPCLGPulseClassifier')
TPCLGPulseClassifier.detector = TPCLG

TPCLGPhotonCounter = PhotonDetectionConf.LZap__PhotonCounter('TPCLGPhotonCounter')
TPCLGPhotonCounter.detector = TPCLG
TPCLGPhotonCounter.constThreshold_phd = 0.002

################################################
# Skin modules
Skin = 'Skin'

SkinPodSummer = PhotonDetectionConf.LZap__PodSummer('SkinPodSummer')
SkinPodSummer.detector = Skin
SkinPodSummer.minSeparationNs = 100

SkinRollingThunder = PhotonDetectionConf.LZap__RollingThunder('SkinRollingThunder')
SkinRollingThunder.detector = Skin

SkinChannelPulseParameterizer = PhotonDetectionConf.LZap__ChannelPulseParameterizer('SkinChannelPulseParameterizer')
SkinChannelPulseParameterizer.detector = Skin

SkinPulseParameterizer = PhotonDetectionConf.LZap__PulseParameterizer('SkinPulseParameterizer')
SkinPulseParameterizer.detector = Skin

SkinPulseClassifier = PhotonDetectionConf.LZap__PulseClassifierHADES('SkinPulseClassifier')
SkinPulseClassifier.detector = Skin

SkinPhotonCounter = PhotonDetectionConf.LZap__PhotonCounter('SkinPhotonCounter')
SkinPhotonCounter.detector = Skin
SkinPhotonCounter.constThreshold_phd = 0.0015

################################################
# OD High Gain modules
ODHG = 'Outer High Gain'

ODHGPodSummer = PhotonDetectionConf.LZap__PodSummer('ODHGPodSummer')
ODHGPodSummer.detector = ODHG
ODHGPodSummer.minSeparationNs = 3000

ODHGRollingThunder = PhotonDetectionConf.LZap__RollingThunder('ODHGRollingThunder')
ODHGRollingThunder.detector = ODHG

ODHGChannelPulseParameterizer1 = PhotonDetectionConf.LZap__ChannelPulseParameterizer('ODHGChannelPulseParameterizer1')
ODHGChannelPulseParameterizer1.detector = ODHG

ODHGPulseParameterizer1 = PhotonDetectionConf.LZap__PulseParameterizer('ODHGPulseParameterizer1')
ODHGPulseParameterizer1.detector = ODHG

ODHGChannelPulseParameterizer = PhotonDetectionConf.LZap__ODChannelPulseParameterizer('ODHGChannelPulseParameterizer')
ODHGChannelPulseParameterizer.detector = ODHG

ODHGPulseParameterizer = PhotonDetectionConf.LZap__ODPulseParameterizer('ODHGPulseParameterizer')
ODHGPulseParameterizer.detector = ODHG

ODHGPulseClassifier = PhotonDetectionConf.LZap__PulseClassifierHADES('ODHGPulseClassifier')
ODHGPulseClassifier.detector = ODHG

ODHGPhotonCounter = PhotonDetectionConf.LZap__PhotonCounter('ODHGPhotonCounter')
ODHGPhotonCounter.detector = ODHG
ODHGPhotonCounter.constThreshold_phd = 0.0015

################################################
# OD Low Gain modules
ODLG = 'Outer Low Gain'

ODLGPodSummer = PhotonDetectionConf.LZap__PodSummer('ODLGPodSummer')
ODLGPodSummer.detector = ODLG
ODLGPodSummer.minSeparationNs = 3000

ODLGRollingThunder = PhotonDetectionConf.LZap__RollingThunder('ODLGRollingThunder')
ODLGRollingThunder.detector = ODLG

ODLGChannelPulseParameterizer1 = PhotonDetectionConf.LZap__ChannelPulseParameterizer('ODLGChannelPulseParameterizer1')
ODLGChannelPulseParameterizer1.detector = ODLG

ODLGPulseParameterizer1 = PhotonDetectionConf.LZap__PulseParameterizer('ODLGPulseParameterizer1')
ODLGPulseParameterizer1.detector = ODLG

ODLGChannelPulseParameterizer = PhotonDetectionConf.LZap__ODChannelPulseParameterizer('ODLGChannelPulseParameterizer')
ODLGChannelPulseParameterizer.detector = ODLG

ODLGPulseParameterizer = PhotonDetectionConf.LZap__ODPulseParameterizer('ODLGPulseParameterizer')
ODLGPulseParameterizer.detector = ODLG

#ODLGPulseClassifier = PhotonDetectionConf.LZap__PulseClassifierHADES('ODLGPulseClassifier')
#ODLGPulseClassifier.detector = ODLG

ODLGPhotonCounter = PhotonDetectionConf.LZap__PhotonCounter('ODLGPhotonCounter')
ODLGPhotonCounter.detector = ODLG
ODLGPhotonCounter.constThreshold_phd = 0.002

################################################
# Combined TPC modules
TPCGainMatcher = PhotonDetectionConf.PhotonDetection__GainMatcher()
TPCGainMatcher.HGdetector = TPCHG
TPCGainMatcher.LGdetector = TPCLG

TPCS2XY = InteractionDetectionConf.InteractionDetection__S2PositionReconstructorMercury()

interactionFinder       = InteractionDetectionConf.InteractionDetection__InteractionFinder()
xyzPositionCorrector    = InteractionDetectionConf.InteractionDetection__XYZPositionCorrector()
photonOverlapCorrector  = InteractionDetectionConf.InteractionDetection__PhotonOverlapCorrector()
pulseAreaCorrector      = InteractionDetectionConf.InteractionDetection__PulseAreaCorrector()

interactionFinder.UseHGClass=True


#################################################
# Define processing chain
commonModules = [ rawEventCorrector,
                  podCalibrator ]
TPCHGModules = [ TPCHGPodSummer,
                 TPCHGRollingThunder,
                 TPCHGChannelPulseParameterizer,
                 TPCHGPulseParameterizer,
                 TPCHGPulseClassifier,
                 TPCHGPhotonCounter ]
TPCLGModules = [ TPCLGPodSummer,
                 TPCLGRollingThunder,
                 TPCLGChannelPulseParameterizer,
                 TPCLGPulseParameterizer,
                 TPCLGPulseClassifier,
                 TPCLGPhotonCounter ]
SkinModules  = [ SkinPodSummer,
                 SkinRollingThunder,
                 SkinChannelPulseParameterizer,
                 SkinPulseParameterizer,
                 SkinPulseClassifier,
                 SkinPhotonCounter ]
ODHGModules  = [ ODHGPodSummer,
                 ODHGRollingThunder,
                 ODHGChannelPulseParameterizer1,
                 ODHGPulseParameterizer1,
                 ODHGChannelPulseParameterizer,
                 ODHGPulseParameterizer ]
ODLGModules  = [ ODLGPodSummer,
                 ODLGRollingThunder,
                 ODLGChannelPulseParameterizer1,
                 ODLGPulseParameterizer1,
                 ODLGChannelPulseParameterizer,
                 ODLGPulseParameterizer ]
TPCModules   = [ TPCGainMatcher,
                 TPCS2XY,
                 interactionFinder,
                 xyzPositionCorrector,
                 photonOverlapCorrector,
                 pulseAreaCorrector ]
app.TopAlg = commonModules + TPCHGModules + TPCLGModules + TPCModules + SkinModules + ODHGModules + ODLGModules