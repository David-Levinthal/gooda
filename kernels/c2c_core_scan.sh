#! /bin/bash
ulimit -n 32768
cores=$(cat /proc/cpuinfo | grep family | wc -l)
echo $cores cores
for i in {2..128..2}; do
	echo ./snoop_test/snoop_test -i0 -r1 -w$i -s $1 $2 $3 $4 $5 $6 $7 $8 $9 >> c2c_latency_scan_s.log 2>&1
	./snoop_test/snoop_test -i0 -r1 -w$i -s $1 $2 $3 $4 $5 $6 $7 $8 $9 >> c2c_latency_scan_s.log 2>&1
done
