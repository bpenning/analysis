#! /usr/bin/env python
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


jobname=0

def process_arguments():
    
    parser = ArgumentParser(description=__doc__, formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-f", "--file", default=None,  required=True, help="Set the input file")
    args = parser.parse_args()
    return args


def submit_jobs(job_script):
    cmd = ["sbatch"]
    cmd += [job_script]
    logger.info("Submitting jobs: '%s'" % " ".join(cmd))
    output = subprocess.check_output(cmd)
    logger.info("sbatch output:\n%s" % output)

    cmd = ["squeue","-l", "-u","penning"]
    output = subprocess.check_output(cmd)
    logger.info("squeue output:\n%s" % output)
    return True


def write_submission_script(jobname, args):
    f1 = open('slurm.sh.tpl', 'r')
    f2 = open(jobname+'.sh', 'w')
    for line in f1:
        line=line.replace('JOBNAME', jobname)
        line=line.replace('INPUTFILE', args.file)
        line=line.replace('DESTINATION', 'out/logs')
        f2.write(line)
    f1.close()
    f2.close()

def check_and_prep():
    if not os.path.isfile(args.file):
        print 'Cannot find file: '+args.file
        sys.exit(0)

    if not os.path.isdir('out/logs'):
        print 'Cannot find the \'logs/out\' directory'
        sys.exit(0)

    global jobname
    jobname=os.path.basename(args.file).replace('.list','')


def main(args):
    check_and_prep()
    write_submission_script(jobname, args)
    submit_jobs(str(jobname)+'.sh')
    os.remove(str(jobname)+'.sh')
    

if __name__ == "__main__":
    args = process_arguments()
    if args:
        main(args)
