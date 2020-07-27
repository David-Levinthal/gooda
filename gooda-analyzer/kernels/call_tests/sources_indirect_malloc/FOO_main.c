//
//
#include <stdio.h>
#include <malloc.h>
#include "FOO.h"

int main(int argc, char* argv[])
{
	long i=0;
	long j,k,m,n=0;
	j=atol(argv[1]);
	m=16;
	FOO_array = (*FOO_pt) malloc(sizeof(FOO_pt)*m);
		FOO_array[n] = &FOO_000_000_000;
		n++;
		FOO_array[n] = &FOO_000_000_001;
		n++;
		FOO_array[n] = &FOO_000_000_002;
		n++;
		FOO_array[n] = &FOO_000_000_003;
		n++;
		FOO_array[n] = &FOO_000_001_000;
		n++;
		FOO_array[n] = &FOO_000_001_001;
		n++;
		FOO_array[n] = &FOO_000_001_002;
		n++;
		FOO_array[n] = &FOO_000_001_003;
		n++;
		FOO_array[n] = &FOO_001_000_000;
		n++;
		FOO_array[n] = &FOO_001_000_001;
		n++;
		FOO_array[n] = &FOO_001_000_002;
		n++;
		FOO_array[n] = &FOO_001_000_003;
		n++;
		FOO_array[n] = &FOO_001_001_000;
		n++;
		FOO_array[n] = &FOO_001_001_001;
		n++;
		FOO_array[n] = &FOO_001_001_002;
		n++;
		FOO_array[n] = &FOO_001_001_003;
		n++;
	for(k=0;k<j;k++){
	n=0;
		FOO_array[n](n);
	n+=4;
		FOO_array[n](n);
	n+=4;
		FOO_array[n](n);
	n+=4;
		FOO_array[n](n);
	n+=4;
	}

	return 0;
}
