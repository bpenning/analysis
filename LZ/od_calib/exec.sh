#./configure.py --force /projecta/projectdirs/lz/data/MDC2/background/LZAP-3.12.0-PHYSICS-3.12.1/20170816/lz_2017081610_lzap.root
./macro.py -n 100 od_calib.C  filelists/background_LZAP-3.12.0-PHYSICS-3.12.1.list
nohup ./macro.py -n 1000 -m 10 --useProof   MyProcessor.C  RQfile_BG_3.10.0_11d.list  >output.dat &



./macro.py -n 10000 -m 10 --useProof od_calib.C filelists/AmLi_1336.list
./macro.py -n 10000 -m 10 --useProof od_calib.C filelists/AmLi_700.list
./macro.py -n 10000 -m 10 --useProof od_calib.Cfilelists/Na22_1336.list
./macro.py -n 10000 -m 10 --useProof od_calib.C filelists/Na22_700.list
./macro.py -n 10000 -m 10 --useProof od_calib.C filelists/Rn220.list
./macro.py -n 10000 -m 10 --useProof od_calib.C filelists/Th228.list
./macro.py -n 10000 -m 10 --useProof od_calib.Cfilelists/background_LZAP-3.12.0-PHYSICS-3.12.1.list