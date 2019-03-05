#!/usr/bin/env python
import os
import sys
import shutil
import ROOT
import argparse

#______________________________________
def load_chain(fileList, chain):
    
    print 'Loading file names from '+fileList+' into '+chain.GetName()
    
    file = open(fileList, 'r')
    rqFiles = file.read().splitlines()
    for line in rqFiles:
        if line.startswith('#'):
            print 'comment: '+line
        else:
            chain.Add(line)

#______________________________________
# define script options
parser = argparse.ArgumentParser(description='Wrapper for TSelector-based analysis on LZ RQ files',formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('macro', type=str, help='TSelector-based analysis macro ( .C file )')
parser.add_argument('fileList', type=str, help='List of RQ files to process')
parser.add_argument('--outfile', default='', type=str, help='Output histogram file name; empty = <fileList>.root')
parser.add_argument('-n', dest='nevents', default=-1, type=int, help='# of events to process; -1 = ALL')
parser.add_argument('--useProof', action='store_true', help='Use PROOF processing')
parser.add_argument('--useGui', action='store_true', help='Show PROOF GUI')
parser.add_argument('-m', dest='nworkers', default=4, type=int, help='# of workers to use for PROOF processing')
args = parser.parse_args()

macro = args.macro
fileList  = args.fileList
outFile = args.outfile
nevents = args.nevents
useProof = args.useProof
nworkers = args.nworkers
useGui = args.useGui

#______________________________________
# decide whether or not to show gui
if useProof and not useGui:
    ROOT.gROOT.SetBatch(True)
    
#______________________________________
# build the Events and RQMCTruth TChains and friend them
chain = ROOT.TChain('Events')
load_chain(fileList, chain)

mcTruth = ROOT.TChain('RQMCTruth')
load_chain(fileList, mcTruth)

chain.AddFriend(mcTruth)

if nevents == -1:
#    nevents = ROOT.TTree.kMaxEntries
    nevents = chain.GetEntries()
    print nevents

#______________________________________
# Do the event processing

# Haven't figured out how to pass output file name through PROOF,
# so do it "manually" through the TSelector
if outFile == '':
    outFile = fileList.rstrip('.list')
    outFile += '.root'
selector=ROOT.TSelector.GetSelector(macro+'+')
selector.SetOutputName(outFile)
print 'Saving outputs to '+outFile

if (args.useProof):

    proof = ROOT.TProof.Open('','workers='+str(nworkers))
    # Load 
    # For ROOT < 6.08.02, PROOF is not loading in the pcm files correctly, so load them manually.
    # See https://sft.its.cern.ch/jira/browse/ROOT-8456
    # and https://sft.its.cern.ch/jira/browse/ROOT-8466
    proof.Load(macro+'+,'+macro[:-2]+'_C_ACLiC_dict_rdict.pcm')
    chain.SetProof(1)
    chain.Process(selector, '', nevents)
    chain.SetProof(0)
else:
    chain.Process(selector, '', nevents)
