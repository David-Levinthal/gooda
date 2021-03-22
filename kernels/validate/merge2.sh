#! /bin/sh
#echo at $@
#echo pound $#
count=1
for f in $@
do
   filename=$f
#   echo filename = $filename
   arr[${count}]=$filename
#   echo "${arr[${count}]}"
   let a=$count+7
   fd[${count}]=$a
#   echo count = $count, fd[count] = ${fd[${count}]}
   eval exec "${fd[${count}]}<$filename"
   let count=count+1
done
#echo after first loop count =  $count
files=1
#echo fd of 1 = ${fd[${files}]}
lines=`wc -l $1 | cut -f1 -d' '`
#echo  lines = $lines
linecount=0

while [ ${linecount} -lt ${lines} ]; do
#   read linefile[${files}] <&${fd[${files}]}
#   a=$linefile[${files}]
   a="@"
#   echo a before read loop = $a
   files=1
   while [ ${files} -lt ${count} ]; do
     read linefile[${files}] <&${fd[${files}]}
     a=$a"@"${linefile[${files}]}
#     echo after file $files a = $a
     let files=files+1
   done
   echo $a
   let linecount=linecount+1
done

