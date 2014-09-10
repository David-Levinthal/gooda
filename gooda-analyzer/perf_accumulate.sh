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
	echo "sum directory 2 is " $2
else
	echo $2 "does not exist"
	exit 2
fi

new_dir=$2+$1
echo "new temp directory (new + old) is" $new_dir
echo $new_dir >> index
mkdir $new_dir
mkdir $new_dir/spreadsheets
gooda_sum.py ./$1/spreadsheets/process.csv ./$2/spreadsheets/process.csv > ./$new_dir/spreadsheets/process.csv
ret=$?
zero=0
echo ret = $reta
if [ $ret -ne $zero ]
	then
		echo "gooda_accumulate.py failed on process.csv"
		exit
fi
gooda_sum.py ./$1/spreadsheets/function_hotspots.csv ./$2/spreadsheets/function_hotspots.csv > ./$new_dir/spreadsheets/function_hotspots.csv
ret=$?
echo ret = $ret
if [ $ret -ne $zero ]
	then
		echo "gooda_accumulate.py failed on function_hotspot.csv"
		exit
fi

cd $2
rm -rf spreadsheets
cp -r ../$new_dir/spreadsheets ./
cd ..
rm -rf $new_dir
