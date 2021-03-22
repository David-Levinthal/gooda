/*
Copyright 2012 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */
#include <stddef.h>

size_t matrix_mult(int len, double *restrict a, double *restrict b, double *restrict c)
{
	int i,j,k,l,m;
        size_t	fpops;
	double temp;
        fpops = (size_t)2*len*len*len;

//transpose matrix b first to create dot product
	
	for(i=0; i< len; i++){
	       for(j=0; j<len; j++){
		       temp = b[i*len+j];
		       b[i*len+j] = b[j*len+i];
		       b[j*len+i] = temp;
	       }
	}


	for(i=0; i< len; i++){
	       for(j=0; j<len; j++){
	       		for(k=0; k<len; k+=4){
		 		a[i*len+j] += b[i*len+k] * c[j*len+k];
		 		a[i*len+j] += b[i*len+k+1] * c[j*len+k+1];
		 		a[i*len+j] += b[i*len+k+2] * c[j*len+k+2];
		 		a[i*len+j] += b[i*len+k+3] * c[j*len+k+3];
			}
	       }
	}
	return fpops;
}
