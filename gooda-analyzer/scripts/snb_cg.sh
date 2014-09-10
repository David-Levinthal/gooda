#!/bin/sh
perf record -j any_ret --pfm-events=unhalted_core_cycles:period=2000000,br_inst_retired:near_return:period=20000:precise=1 -a -R
