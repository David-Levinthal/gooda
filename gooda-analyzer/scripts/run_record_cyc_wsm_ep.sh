#!/bin/sh
ulimit -n 32768
DIR=GOODA_DIR
perf record  $(grep -v '^$' $DIR/scripts/wsm_ep_cyc_account_only.txt | grep -v ^# | sed -e 's/^/--pfm-events /') -a -R  -- $1 $2 $3 $4

