#! /bin/sh
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

./FOO_static -i$1 -r$2 -L$3
