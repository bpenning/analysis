#! /usr/bin/env python
"""
Submit DER jobs to SLURM
"""
import os
import sys
from datetime import datetime
import logging
import yaml
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import re
from exceptions import OSError
import ntpath
import subprocess
import errno
import textwrap
import shutil


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


NOW = datetime.now()
jobname=0

def process_arguments():
    parser = ArgumentParser(description=__doc__, formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o", "--out-dir", default=None,  required=True, help="Set the output directory")
    parser.add_argument("-f", "--file", default=None,  required=True, help="Set the input file")
    args = parser.parse_args()
    return args


def submit_jobs(job_script):
    cmd = ["sbatch"]
#    cmd = ["ls"]
    cmd += [job_script]
    logger.info("Submitting jobs: '%s'" % " ".join(cmd))
    output = subprocess.check_output(cmd)
    logger.info("sbatch output:\n%s" % output)

    cmd = ["squeue","-l", "-u","penning"]
    output = subprocess.check_output(cmd)
    logger.info("squeue output:\n%s" % output)
    return True


def write_submission_script(jobname, args):
    f1 = open('submit_DER.sh.tpl', 'r')
    f2 = open(jobname+'.sh', 'w')
    for line in f1:
        line=line.replace('JOBNAME', jobname)
        line=line.replace('INPUTFILE', args.file)
        line=line.replace('DESTINATION', args.out_dir+'/')
        f2.write(line)
    f1.close()
    f2.close()

def check_and_prep():
    #use absolute paths
    args.file=os.path.abspath(args.file)
    args.out_dir=os.path.abspath(args.out_dir)

    #exit if file/dir not found
    if not os.path.exists(args.file):
        print 'Cannot find file: '+args.file
        sys.exit(0)
    if not os.path.exists(args.out_dir):
        print 'Cannot find dir: '+args.out_dir
        sys.exit(0)
    #set name for global variable
    ntpath.basename("a/b/c")
    head, tail = ntpath.split(args.file)
    global jobname
    jobname = tail.replace('.root','')
    jobname = jobname.replace('Tutorial','bp')
        

def main(args):
    check_and_prep()
    job_script=jobname+'.sh'
    write_submission_script(jobname, args)
    submit_jobs(job_script)
    os.remove(job_script)
    

if __name__ == "__main__":
    args = process_arguments()
    if args:
        main(args)
