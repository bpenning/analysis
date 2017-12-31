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
#SETUP_BACCARAT_ON_CVMFS = "/cvmfs/lz.opensciencegrid.org/BACCARAT/"
#CONFIG_FILE = "config.yaml"
#SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))
#DEFAULT_TEMPLATE_JOBSCRIPT = os.path.join(SCRIPTS_DIR, "template_job_script.sh.tpl")


def process_arguments():
    parser = ArgumentParser(description=__doc__, formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o", "--out-dir", default=None,  help="Set the output directory")
    parser.add_argument("-f", "--file", default=None,  help="Set the input file")
    args = parser.parse_args()
    return args


def submit_jobs(job_script):
    cmd = ["sbatch"]
#    cmd = ["ls"]
    cmd += [job_script]
    print cmd
    logger.info("Submitting jobs: '%s'" % " ".join(cmd))
    output = subprocess.check_output(cmd)
    logger.info("sbatch output:\n%s" % output)

    cmd = ["squeue","-l", "-u","penning"]
    output = subprocess.check_output(cmd)
    logger.info("squeue output:\n%s" % output)
    return True


def write_submissoin_script(args):
    f1 = open('submit_DER.sh.tpl', 'r')
    f2 = open('submit_DER.sh', 'w')
    
    ntpath.basename("a/b/c")
    head, tail = ntpath.split(args.file)
    print 'bp '+ head
    jobname = tail.replace('.root','')
    jobname = jobname.replace('Tutorial','bp')
    for line in f1:
        line=line.replace('JOBNAME', jobname)
        line=line.replace('INPUTFILE', args.file)
        line=line.replace('DESTINATION', args.out_dir+'/')
        f2.write(line)
    f1.close()
    f2.close()


def main(args):
    print '-------------'
    args.file=os.path.abspath(args.file)
    args.out_dir=os.path.abspath(args.out_dir)


    job_script='submit_DER.sh'

    write_submissoin_script(args)

    submit_jobs(job_script)
    os.remove(job_script)

#dont' forget
#source /cvmfs/lz.opensciencegrid.org/BACCARAT/release-2.4.0/setup.sh

#    batch_dir = prepare_batch_directory(**vars(args))
#    if not batch_dir:
#        return False
#    logger.info("Batch output will be stored under: " + batch_dir)

    # Log the config of things
#    config = write_submision_configuration(batch_dir, **vars(args))

    # Prepare job-script
#    job_script = write_job_script(batch_dir, **vars(args))

    # Submit jobs
#    ok = submit_jobs(job_script, **vars(args))
#    return ok


if __name__ == "__main__":
    args = process_arguments()
    if args:
        main(args)
