#! /bin/sh
echo $1
echo $2
echo $3

lines1=`wc -l $1 | cut -f1 -d' '`
echo $lines1
lines2=`wc -l $2 | cut -f1 -d' '`
echo $lines2
lines3=`wc -l $3 | cut -f1 -d' '`
echo $lines3

count1=1
count2=1
count3=1
let lines1=lines1+1
let lines2=lines2+1
let lines3=lines3+1
while [ $count1 -lt $lines1 ]; do
	head -n $count1 $1 | tail -n 1
	let count1=count1+1
	if [ "$count2" -lt "$lines2" ]; then
		head -n $count2 $2 | tail -n 1
		let count2=count2+1
	fi
	head -n $count1 $1 | tail -n 1
	let count1=count1+1
	if [ "$count3" -lt "$lines3" ]; then
		head -n $count3 $3 | tail -n 1
		let count3=count3+1
	fi
done
