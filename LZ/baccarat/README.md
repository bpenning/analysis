# BACCARAT submission

Description how to run all BACCARAT steps on `PDFS` and hopefully also `CORI`

## Getting Started

Make sure to have a physics environment sources

```
source /cvmfs/lz.opensciencegrid.org/Physics/latest/Physics/setup.sh
```


### Example how to run single example

To run one example by hand please see:


```
source /cvmfs/lz.opensciencegrid.org/Physics/latest/Physics/setup.sh
export OUTPUT_DIR=bla_
source /cvmfs/lz.opensciencegrid.org/BACCARAT/latest/x86_64-slc6-gcc48-opt/setup.sh
/cvmfs/lz.opensciencegrid.org/BACCARAT/latest/x86_64-slc6-gcc48-opt/bin/BACCARATExecutable lz_mdc1_calib_Co57.mac
/cvmfs/lz.opensciencegrid.org/BACCARAT/latest/x86_64-slc6-gcc48-opt/bin/BaccRootConverter bla_/bla_2061566397.bin
/cvmfs/lz.opensciencegrid.org/BACCARAT/latest/x86_64-slc6-gcc48-opt/bin/BaccMCTruth bla_/bla_2061566397.root
source  /cvmfs/lz.opensciencegrid.org/DER/latest/x86_64-slc6-gcc48-opt/setup.sh
DER --source bla_/bla_2061566397_mctruth.root
lzap /global/project/projectdirs/lz/releases/physics/latest/ProductionSteeringFiles/RunLZapMDC2TimeIntervals.py

```


## Running on slurm


```
./baccarat_via_slurm.py -n 3 -e 1000 -o out -T latest/x86_64-slc6-gcc48-opt/ lz_mdc1_calib_Co57.mac

```


### Now pipping through DER

