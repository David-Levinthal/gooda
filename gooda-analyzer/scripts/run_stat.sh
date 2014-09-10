#!/bin/sh
ulimit -n 32768
DIR=GOODA_DIR
perf stat  $(grep -v '^$' $DIR/scripts/wsm-event1.txt | grep -v ^# | sed -e 's/^/--pfm-events /') -a -x @ -- $1 $2 $3 $4
perf stat  $(grep -v '^$' $DIR/scripts/wsm-event2.txt | grep -v ^# | sed -e 's/^/--pfm-events /') -a -x @ -- $1 $2 $3 $4
perf stat  $(grep -v '^$' $DIR/scripts/wsm-event3.txt | grep -v ^# | sed -e 's/^/--pfm-events /') -a -x @ -- $1 $2 $3 $4

