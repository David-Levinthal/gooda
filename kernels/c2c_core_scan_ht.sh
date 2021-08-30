#! /bin/bash
ulimit -n 32768
cores=$(cat /proc/cpuinfo | grep family | wc -l)
((cores=cores/2-1))
echo $cores cores
for i in $( eval echo "{2..$cores}"); do
	echo ./snoop_test/snoop_test -i0 -r1 -w$i $1 $2 $3 $4 $5 $6 $7 $8 $9 >> c2c_latency_scan_4.test 2>&1
	./snoop_test/snoop_test -i0 -r1 -w$i $1 $2 $3 $4 $5 $6 $7 $8 $9 >> c2c_latency_scan_4.log 2>&1
done
