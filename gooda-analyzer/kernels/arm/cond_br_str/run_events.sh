#! /bin/bash
perf stat -a -A --pfm-events cycles,inst_retired,br_retired,br_mis_pred_retired -- ./cond_br_driver_str -i0 -r2 -l500000 -m10000 > br_test_l500k_m1K_O3_T0 2>&1
