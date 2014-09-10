#! /bin/sh

perf record -j any_ret --pfm-events br_inst_retired:near_return:period=40009 -a -R -- $1 $2 $3 $4 $5 $6 $7 $8 $9
