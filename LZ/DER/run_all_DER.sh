mkdir $1/DER
for f in $1/raw/job_*/*mctruth.root; do echo ./submit_DER.py -o $1/DER/ -f $f; done

