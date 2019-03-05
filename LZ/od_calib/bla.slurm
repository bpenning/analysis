#!/bin/bash -l
#SBATCH -q debug
#SBATCH -N 2
#SBATCH -t 01:00:00
#srun -n 48 ./my_executable

srun -n 1 ./macro.py -n 1000 MyProcessor.C RQfile_BG_3.10.0_35d.list
