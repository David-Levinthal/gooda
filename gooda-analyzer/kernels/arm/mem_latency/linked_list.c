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

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>

size_t * reader(int len, size_t * buf1)
{
	int count;
	size_t* p;
	count = 0;
	p = buf1;

	while(count < len)
		{
		p = (size_t *)*p;
		count++;
		}
	return p;
}

