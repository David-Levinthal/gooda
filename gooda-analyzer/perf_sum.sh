#! /bin/sh

if [ -d "$1" ]
then
	echo "old directory 1 is " $1
else
	echo $1 "does not exist"
	exit 1
fi

if [ -d "$2" ]
then
	echo "new directory 2 is " $2
else
	echo $2 "does not exist"
	exit 2
fi

new_dir=$2+$1
echo "new directory (new + old) is" $new_dir
mkdir $new_dir
mkdir $new_dir/spreadsheets
gooda_sum.py ./$1/spreadsheets/process.csv ./$2/spreadsheets/process.csv > ./$new_dir/spreadsheets/process.csv
ret=$?
zero=0
echo "ret = " $ret
if [ $ret -ne $zero ]
    then
	echo "gooda_sum.py failed on process.csv"
        exit
fi
gooda_sum.py ./$1/spreadsheets/function_hotspots.csv ./$2/spreadsheets/function_hotspots.csv > ./$new_dir/spreadsheets/function_hotspots.csv
ret=$?
echo "ret = " $ret
if [ $ret -ne $zero ]
    then
	echo "gooda_sum.py failed on function_hotspots.csv"
        exit
fi
echo $new_dir >> index
