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
        fpops = (size_t)2*len*len*len;

	for(i=0; i< len; i++){
	       for(j=0; j<len; j++){
	       		for(k=0; k<len; k++){
		 		a[i*len+k] += b[i*len+j] * c[j*len+k];
			}
	       }
	}
	return fpops;
}
