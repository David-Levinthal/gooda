//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "triad.h"

long triad(int len, char* str_arg, char* str_arg2, double *restrict a, double *restrict b, double *restrict c)
{
	 int i,bytes=16;
	 if(dumb_compare(str_arg,str_arg2) == 0){
	         bytes = FOO1(a,b,c);
	 }
	 else {
	         bytes = FOO2(a,b,c);
	 }
	 return bytes;
}
