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
#SBATCH --time=6:00:00
#################
#Choose a partition
#Options for Cori: 'debug' (runtime<30m, quick start), 'regular' (max runtime of 2-3days, long wait queue)
#Options for PDSF: 'shared', 'shared-chos' (only use the latter for now)
##SBATCH --partition=shared-chos
#################
#OPTIONAL
#################
#Job name
#SBATCH --job-name={job_name}
#################  
#File for job output. Append the job ID with %j to make it unique
#SBATCH --output={output_file}
#################
#File for job errors 
#SBATCH --error={error_file}
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

echo "Modules are:"
module load qt/5.5.0
module list

date
export OUTPUT_DIR="{bin_dir}/job_$SLURM_JOB_ID"

echo "Setting up BACCARAT for job SLURM_JOB_ID = $SLURM_JOB_ID"
source {bacc_setup_script}

echo "----- START Environment ------"
printenv
echo "----- STOP Environment ------"

echo "Reset TMPDIR to point to SLURM_TMP so BACCARAT puts it's history commands there instead"
TMPDIR="$SLURM_TMP"

echo "Change to BACCARAT directory: '$BACC_ROOT'"
cd "$BACC_ROOT"
pwd 
ls -l

echo "Output of `lcdonfig -p`"
ldconfig -p

# Find all executables:
function FindExecutable(){{
    local executable="$(find . -name "$1" -type f -executable |head -1)"
    if [ -z "$executable" ];then
        echo "Cannot find executable: '$1'" >&2
        echo "Terminating..." >&2
        exit
    fi
    echo "$executable"
}}
BACCARATExecutable=$(FindExecutable BACCARATExecutable)
BaccRootConverter=$(FindExecutable BaccRootConverter)
BaccMCTruthConverter=$(FindExecutable BaccMCTruth)

echo "Running BACCARAT"
echo "Outputting to '$OUTPUT_DIR'"
date
"$BACCARATExecutable" {run_macro}
date
echo "Finished BACCARAT"

echo "Temprorary: Contained in /tmp"
ls -l /tmp
echo "Temprorary: Contained in SLURM_TMP=$SLURM_TMP"
ls -l $SLURM_TMP

# Convert the .bin file to a .root file
echo "Starting converter"
"$BaccRootConverter" "$OUTPUT_DIR/"*.bin

# Convert the .root file to a _mctruth.root file
echo "Starting MCTruth converter"
"$BaccMCTruthConverter" "$OUTPUT_DIR/"*.root

# Move the root files to a common folder
root_file="$(echo "$OUTPUT_DIR"/*.root)"
base_name="$(basename "$root_file")"
root_dest_file="{out_dir}/job-${{SLURM_JOB_ID}}_$(printf "%03d" ${{SLURM_ARRAY_TASK_ID:-0}})-$base_name"
echo "Moving root file: FROM '$root_file' TO '$root_dest_file'"
mv "$root_file" "$root_dest_file"

# Can now delete the .bin file
if [ $? == 0 ];then
# This combination of options:
# Won't fail if file doesn't exist, but
# doesn't prompt for removal if rm has been aliased to do so
rm -f --interactive=never "$OUTPUT_DIR/"*.bin
fi
cd -

# Should probably write first to a scratch volume and then copy to final place...
