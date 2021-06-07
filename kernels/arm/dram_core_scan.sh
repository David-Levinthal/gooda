#! /bin/bash
ulimit -n 32768
cores=$(cat /proc/cpuinfo | grep processor | wc -l)
((cores=cores-1))
echo $cores cores
for i in $( eval echo "{1..$cores}"); do
	./mem_latency/walker -i0 -r$i $1 $2 $3 $4 $5 $6 $7 $8 $9 >> latency_scan.log 2>&1

done
