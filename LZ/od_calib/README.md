
#Performing OD calib analysis 


Steps to run:
```
./configure.py --force /path/sampleDataFile.root (only have to do once)
./macro.py od_calib.C fileList.list (for optional arguements, ./macro.py -h)

```
## to submit to queue ##
```
./submit_od.py -f filelists/Na22_1336.list
```