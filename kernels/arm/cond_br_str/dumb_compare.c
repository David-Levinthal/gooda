#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int dumb_compare(char* s1, char*s2)
{
	int i,j,k, retval;
	retval = (s1[0]-s2[0]);
	retval += (s1[1]-s2[1]);
	retval += (s1[2]-s2[2]);
	retval += (s1[3]-s2[3]);
	retval += (s1[4]-s2[4]);
	retval += (s1[5]-s2[5]);
	retval += (s1[6]-s2[6]);
	retval += (s1[7]-s2[7]);
	retval += (s1[8]-s2[8]);
	retval += (s1[9]-s2[9]);
	retval += (s1[10]-s2[10]);
	retval += (s1[11]-s2[11]);
	retval += (s1[12]-s2[12]);
	retval += (s1[13]-s2[13]);
	retval += (s1[14]-s2[14]);
	retval += (s1[15]-s2[15]);
	return retval;
}
