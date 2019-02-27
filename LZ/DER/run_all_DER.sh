for f in raw/job_*/*mctruth.root; do echo ./submit_DER.py -o DER/ -f $f; done
