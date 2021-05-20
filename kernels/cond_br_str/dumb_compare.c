#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int dumb_compare(char* s1, char*s2)
{
	int i,j,k, retval,len1,len2;
	len1 =strlen(s1);
	len2 =strlen(s2);
        if(len1 != len2){
		fprintf(stderr, "from dumb_compare..len1 = %d, does not equal len2 = %d\n",len1,len2);
		exit(1);
		}
	retval = 0;
	for(i=0;i<len1; i++) retval +=(s1[i] - s2[i]);
	return retval;
}
