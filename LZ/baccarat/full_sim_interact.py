#! /usr/bin/env python

import os, sys, glob, subprocess, shlex
import pprint
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter


bac_source='source /cvmfs/lz.opensciencegrid.org/BACCARAT/release-4.3.7/x86_64-slc6-gcc48-opt/setup.sh &&' 
der_source='source  /cvmfs/lz.opensciencegrid.org/DER/latest/x86_64-slc6-gcc48-opt/setup.sh &&' 
lzap_source='source /cvmfs/lz.opensciencegrid.org/Physics/release-3.13.9/Physics/setup.sh &&'

if  len(sys.argv)!=5:
    print 'usage: full_sim_interact.py  -i <input macro> -o <output dir>'
    sys.exit()

def process_arguments():
    parser = ArgumentParser(description=__doc__, formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o", "--out-dir", default=None, 
                        help="Set the output directory")
    parser.add_argument("-i", "--input-file", default=None, 
                        help="Set the input macro file")
    args = parser.parse_args()
    return args

def check_and_prepare(args):
    if not os.path.isdir(args.out_dir):
        os.mkdir(args.out_dir)
        print 'Created directory '+args.out_dir

    if not os.path.exists(args.input_file):
        print 'Error '+args.input_file+' does not exist'
        sys.exit(0)

def run_baccarat(args):
    print '=== Running now BACCARAT ==='
 #   my_env = os.environ
 #   my_env['OUTPUT_DIR'] = os.path.abspath(args.out_dir)
#    print my_env
#    subprocess.Popen(["echo", "$PWD"], env={'OUTPUT_DIR':'test/'})
#    command = shlex.split("env -i bash -c 'source /cvmfs/lz.opensciencegrid.org/BACCARAT/release-4.3.7/x86_64-slc6-gcc48-opt/setup.sh'")
#    print command
    cmd=bac_source+'export OUTPUT_DIR='+os.path.abspath(args.out_dir)+ ' && '
    cmd+='echo $OUTPUT_DIR &&'
    cmd+='BACCARATExecutable '+args.input_file
    print cmd
    subprocess.call(cmd, shell=True)
    list_of_files = glob.glob(args.out_dir+'/*.bin')
    latest_file = max(list_of_files, key=os.path.getctime)
    print latest_file
    cmd=bac_source+'BaccRootConverter '+latest_file
    subprocess.call(cmd, shell=True)
    list_of_files = glob.glob(args.out_dir+'/*.root')
    latest_file = max(list_of_files, key=os.path.getctime)
    print latest_file
    cmd=bac_source+'BaccMCTruth '+latest_file
    subprocess.call(cmd, shell=True)

    return True

#BaccMCTruth Tutorial_1931151380.root
#    proc = subprocess.Popen(command)
#    for line in proc.stdout:
#        (key, _, value) = line.partition("=")
#        os.environ[key] = value
#        proc.communicate()
#    print proc.stdout
#    pprint.pprint(dict(os.environ))
    

def run_der(args):
    print '=== Running now DER ==='
    list_of_files = glob.glob(args.out_dir+'/*.root')
    latest_file = max(list_of_files, key=os.path.getctime)
    cmd=der_source+' DER --source '+latest_file+' --outDir '+os.path.abspath((args.out_dir))
    print cmd
    subprocess.call(cmd, shell=True)
    
    return True


def run_lzap(args):
    print '=== Running LZAP ==='
    cmd=lzap_source+' export LZAP_INPUT_FILES='+max(glob.glob(os.path.abspath((args.out_dir))+'/*raw.root'), key=os.path.getctime) 
    cmd+=' && export LZAP_OUTPUT_FILES=bla.root && cd '+os.path.abspath((args.out_dir))+'  &&'
    cmd+='/cvmfs/lz.opensciencegrid.org/LZap/release-3.13.9/LZap/scripts/lzap_execute ../RunLZapODModules_py.py'
    print cmd
    subprocess.call(cmd, shell=True)
    
    return True

def main(args):
    process_arguments()
    check_and_prepare(args)
    run_baccarat(args)
    run_der(args)
    run_lzap(args)
    


if __name__ == "__main__":
    args = process_arguments()
    if args:
        main(args)

