>># Create MC Truth NTuple



## Compile


```
g++ -I /usr/common/usg/software/ROOT/6.04.00/include/ `root-config --cflags --libs` -o MCTruth_ntuple_create MCTruth_ntuple_create.cc /cvmfs/lz.opensciencegrid.org/BACCARAT/release-4.3.7/x86_64-slc6-gcc48-opt/lib/libBaccMCTruthEvent.so
```


## Set LD_LIBRARY_PATH

Need to be set to be able to execute


```
export LD_LIBRARY_PATH=/cvmfs/lz.opensciencegrid.org/BACCARAT/release-4.3.7/x86_64-slc6-gcc48-opt/lib/:$LD_LIBRARY_PATH
```


## execute

```
./MCTruth_ntuple_create input.root output.root
```