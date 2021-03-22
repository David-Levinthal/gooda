#! /bin/sh
ulimit -n 32768

lines=`wc -l $1 | cut -f1 -d' '`
echo $lines
count=4
while [ $count -lt $lines ]; do
#!	a=`head -n 14 $1 | tail -n 4`
#!	echo $a
	a=`head -n $count $1 | tail -n 4 | tr '[:space:]' ','`
#!	echo $a
#!	echo "${a%?}"
	b="${a%?}"
	echo $b
perf stat -e $b -a -C $2 -x @ -- $3 $4 $5 $6 $7 $8 $9
	let count=count+4
done
let count=count-4
let remain=lines-count
if [ "$remain" -gt 0 ]; then
	a=`head -n $lines $1 | tail -n $remain | tr '[:space:]' ','`
#!	echo "${a%?}"
	b="${a%?}"
	echo $b
perf stat -e $b -a -C $2 -x @ -- $3 $4 $5 $6 $7 $8 $9
fi
