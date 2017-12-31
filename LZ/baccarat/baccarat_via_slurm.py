#! /usr/bin/env python
"""
Submit BACCARAT jobs to SLURM
"""
import os
import sys
from datetime import datetime
import logging
import yaml
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import re
from exceptions import OSError
import errno
import textwrap
import subprocess
import shutil


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


NOW = datetime.now()
SETUP_BACCARAT_ON_CVMFS = "/cvmfs/lz.opensciencegrid.org/BACCARAT/"
CONFIG_FILE = "config.yaml"
SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))
DEFAULT_TEMPLATE_JOBSCRIPT = os.path.join(SCRIPTS_DIR, "template_job_script.sh.tpl")


def process_arguments():
    latest_tag = get_latest_bacc_tag()
    parser = ArgumentParser(description=__doc__, formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-e", "--events-per-job", type=int, default=100, 
                        help="Set the number of events per job")
    parser.add_argument("-n", "--n-jobs", type=int, default=10, 
                        help="Set the number of jobs to submit")
    parser.add_argument("-m", "--max-jobs", type=int, default=None, 
                        help="Set the maximum number of jobs to run at once")
    parser.add_argument("-o", "--out-dir", default=None, 
                        help="Set the output directory")
    parser.add_argument("-s", "--bacc-setup-script", default=None,
                        help="A setup script to configure BACCARAT in each job")
    parser.add_argument("-T", "--bacc-tag", default=latest_tag,
                        help="The tag to use for BACCARAT, under CVMFS")
    parser.add_argument("-j", "--job-script-template", default=DEFAULT_TEMPLATE_JOBSCRIPT,
                        help="A template jobscript that this code fills in.")
    parser.add_argument("--job-name", default=None,
                        help="Set the job name")
    parser.add_argument("macro", help="The path to a valid BACCARAT macro to run")
    args = parser.parse_args()

    # Tweak / resolve complex args
    args.macro = os.path.realpath(args.macro)
    if not args.bacc_setup_script:
        args.bacc_setup_script = os.path.join(SETUP_BACCARAT_ON_CVMFS, args.bacc_tag, "setup.sh")
    else:
        args.bacc_setup_script = os.path.realpath(args.bacc_setup_script)
        args.bacc_tag = None
    if not os.path.isfile(args.bacc_setup_script):
        logger.error("Cannot read setup script for BACCARAT, '%s'" % args.bacc_setup_script)
        return None
    if not args.job_name:
        args.job_name = os.path.basename(os.path.splitext(args.macro)[0])

    return args


def get_latest_bacc_tag():
    tag_form = r"release-(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)"
    tag_re = re.compile(tag_form)
    latest_tag = None
    tag_paths = os.listdir(SETUP_BACCARAT_ON_CVMFS)
    for path in tag_paths:
        if not os.path.isdir(os.path.join(SETUP_BACCARAT_ON_CVMFS, path)):
            continue
        tag_match = tag_re.match(path)
        if not tag_match:
            continue
        version = map(int, tag_match.groups())
        update = False
        if not latest_tag:
            update = True
        else:
            for current, new in zip(latest_tag[0], version):
                if new > current:
                    update = True
                    break
                elif new < current:
                    break

        if update:
            latest_tag = (version, path)
    if latest_tag:
        return latest_tag[1]
    return None


def get_unique_out_dir(outdir=None, revision=1):
    full_outdir = outdir + "-{rev}".format(rev=revision)
    if os.path.isdir(full_outdir):
        return get_unique_out_dir(outdir, revision + 1)
    return full_outdir


def prepare_batch_directory(out_dir, macro, job_name, **kwargs):
    # Make sure we have an output directory
    if not out_dir:
        out_dir = os.path.join(os.getcwd(), "batch_output")
        try:
            os.mkdir(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise 
    elif not os.path.isdir(out_dir):
        logger.error("Cannot find requested output directory: " + out_dir)
        return False
    out_dir = os.path.realpath(out_dir)

    # Get a unique output sub-directory:
    batch_dir = os.path.join(out_dir, "{0:%y}{0:%m}{0:%d}_{1}".format(NOW, job_name))
    batch_dir = get_unique_out_dir(batch_dir)
    os.mkdir(batch_dir)

    for subdir in ["logs", "raw", "out"]:
        os.mkdir(os.path.join(batch_dir, subdir))

    return batch_dir


def write_submision_configuration(batch_dir, **args):
    config = {}
    config["command_line"] = sys.argv
    config["processed_cmd_args"] = args
    config["batch_dir"] = batch_dir
    config["launch_time"] = NOW
    config["run_dir"] = os.getcwd()
    with open(os.path.join(batch_dir, CONFIG_FILE), "w") as out_file:
        yaml.dump(config, out_file, default_flow_style=False)
    return config


def write_job_script(batch_dir, job_script_template, bacc_setup_script, 
                     macro, events_per_job, job_name, **args):
    # Copy the requested macro to the batch_dir
    in_macro = macro
    macro = os.path.join(batch_dir, os.path.basename(in_macro))
    shutil.copyfile(in_macro, macro)

    # Write the control macro:
    macro_contents = textwrap.dedent("""\
        /control/alias OUTPUT_DIR empty
        /control/getEnv OUTPUT_DIR
        /Bacc/io/outputDir {{OUTPUT_DIR}}

        /control/execute {macro}

        /Bacc/beamOn {events_per_job}
        exit
        """)
    macro_contents = macro_contents.format(events_per_job=events_per_job, macro=macro)

    job_macro_path = os.path.join(batch_dir, "BACC_on_slurm.mac")
    with open(job_macro_path, "w") as job_macro:
        job_macro.write(macro_contents)

    # Load the template batch script
    with open(job_script_template, "r") as tpl_file:
        template = tpl_file.read()

    # Format things for submission
    log_dir = os.path.join(batch_dir, "logs")
    bin_dir = os.path.join(batch_dir, "raw")
    out_dir = os.path.join(batch_dir, "out")
    log_file = os.path.join(log_dir, "slurm_job-%j-%3a.log")
    job_script_contents = template.format(log_dir=log_dir,
                                          bin_dir=bin_dir,
                                          out_dir=out_dir,
                                          job_name=job_name,
                                          output_file=log_file,
                                          error_file=log_file,
                                          bacc_setup_script=bacc_setup_script,
                                          run_macro=job_macro_path,
                                          )

    # Write to disk
    job_script_path = os.path.join(batch_dir, "BACC_on_slurm.sh")
    with open(job_script_path, "w") as job_script:
        job_script.write(job_script_contents)

    return job_script_path


def submit_jobs(job_script, n_jobs, max_jobs, **args):
    cmd = ["sbatch"]
    if n_jobs > 1:
        array_spec = "1-{}".format(n_jobs)
        if max_jobs and max_jobs > 0:
            array_spec += "%{}".format(max_jobs)
        cmd += ["-a", array_spec]
    cmd += [job_script]
    logger.info("Submitting jobs: '%s'" % " ".join(cmd))
    output = subprocess.check_output(cmd)
    logger.info("sbatch output:\n%s" % output)

    cmd = ["squeue","-l", "-u","penning"]
    output = subprocess.check_output(cmd)
    logger.info("squeue output:\n%s" % output)
    return True


def main(args):
    batch_dir = prepare_batch_directory(**vars(args))
    if not batch_dir:
        return False
    logger.info("Batch output will be stored under: " + batch_dir)

    # Log the config of things
    config = write_submision_configuration(batch_dir, **vars(args))

    # Prepare job-script
    job_script = write_job_script(batch_dir, **vars(args))

    # Submit jobs
    ok = submit_jobs(job_script, **vars(args))
    return ok


if __name__ == "__main__":
    args = process_arguments()
    if args:
        main(args)
