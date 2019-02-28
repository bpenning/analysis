for f in $1/DER/*/*raw.root; do echo ./submit_LZAP.py -o $1/LZAP/ -f $f; done

