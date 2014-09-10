#! /bin/sh

gcc -static power_drive.c -o power_drive -lpthread
gcc -static power_drive2.c -o power_drive2 -lpthread

