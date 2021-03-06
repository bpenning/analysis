#! /bin/bash
#All commands that start with #SBATCH contain commands that are used by SLURM for scheduling
#Do NOT leave any black line before any of the #SBATCH directives
#################
#REQUIRED
#################
#Number of requested nodes 
#1 node = several cores
#the more you ask, the longer you wait
#Default: '1'
#SBATCH --nodes=1
#################
#Maximum walltime allowed for the job
#Format: dd-hh:mm:ss, hh:mm:ss, mm:ss or mm
#Default: '2:00:00'
#SBATCH --time=2:00:00
#################
#Choose a partition
#Options for Cori: 'debug' (runtime<30m, quick start), 'regular' (max runtime of 2-3days, long wait queue)
#Options for PDSF: 'shared', 'shared-chos' (only use the latter for now)
##SBATCH --partition=shared-chos
#################
#OPTIONAL
#################
#Job name
#SBATCH --job-name=JOBNAME
#################  
#File for job output. Append the job ID with %j to make it unique
#SBATCH --output=DESTINATION/JOBNAME.log
#################
#File for job errors 
#SBATCH --error=DESTINATION/JOBNAME.err
#################
#NERSC repository
#SBATCH --account=lz
#################
#Quality of Service (QOS)
#use this to send jobs to a special queue
#Default: 'normal', which corresponds to a maximum of 48h
#Options: 'normal', 'dev', 'bigmem', 'gpu', 'long'
#more details: http://sherlock.stanford.edu/mediawiki/index.php/Current_policies#Sherlock_Queue_Structure
#SBATCH --qos=normal
#################
#Requested memory per node
#Default: 4G per CPU. You can either ask for more CPUs or more memory (automatically allocated)
#use this specially if your job requires more than 1G of memory
#Request max RAM (ask for 20% more in general to be safe):
#SBATCH --mem=2G
#Request cores:
##SBATCH --ntasks=4
#################

# Add a check that things are running efficiently,
# suggested by Jan Balewski from NERSC
# Waits two minutes, then calls top in batch mode and dumps it once.  This
# happens in the background so we progress with the rest of the script in the
# meantime
(sleep 1200; top ibn1 )&

#echo "Modules are:"
#module load qt/5.5.0
#module list

date

export input_file=INPUTFILE
export destination=DESTINATION
#source /cvmfs/lz.opensciencegrid.org/BACCARAT/release-2.6.0/setup.sh
source  /cvmfs/lz.opensciencegrid.org/DER/latest/x86_64-slc6-gcc48-opt/setup.sh
pwd
ls -alrth
#cd /cvmfs/lz.opensciencegrid.org/DER/release-4.4.0/
DER --source $input_file --outDir $destination
#./DER --UserCheck false --outDir $destination $input_file
#cd -
