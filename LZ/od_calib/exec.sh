#./configure.py --force /projecta/projectdirs/lz/data/MDC2/background/LZAP-3.10.0-PHYSICS-3.10.0/20170507/lz_2017050723_lzap.root
./macro.py -n 100 --useProof  MyProcessor.C  RQfile_BG_3.10.0_11d.list

nohup ./macro.py -n 1000 -m 10 --useProof   MyProcessor.C  RQfile_BG_3.10.0_11d.list  >output.dat &
