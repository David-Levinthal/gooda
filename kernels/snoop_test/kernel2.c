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

#include <sys/mman.h>
#include <asm/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <getopt.h>

typedef size_t * size_t_ptr;

size_t_ptr read_buf(int len, size_t * buf1)
{
	int counter;
	size_t_ptr p;
	counter = 0;
	p = (size_t_ptr) buf1;
	while(counter < len)
		{
		p =(size_t_ptr) *p;
		counter++;
		}
	return p;
}

size_t_ptr write_buf(int len, size_t * buf1)
{
	int counter;
	size_t_ptr p;
	counter = 0;
	p = (size_t_ptr) buf1;
	while(counter < len)
		{
		p = (size_t_ptr) *p;
		*(p+2) = 0;
		counter++;
		}
	return p;
}
int read_flush_buf(int len, size_t * buf1)
{
        unsigned int counter;
        asm(
#ifdef L32
        "movl %1, %%eax  \n\t"
        "movl %2, %%ecx  \n\t"
        "xorl %%ebx, %%ebx \n\t"
        "LOOP: \n\t"
        "inc %%ebx \n\t"
        "mov (%%eax), %%edx \n\t"
	"clflush (%%eax) \n\t"
	"mov %%edx, %%eax \n\t"
#else
        "movq %1, %%rax  \n\t"
        "movl %2, %%ecx  \n\t"
        "xorl %%ebx, %%ebx \n\t"
        "LOOP: \n\t"
        "inc %%ebx \n\t"
        "mov (%%rax), %%rdx \n\t"
	"clflush (%%rax) \n\t"
	"mov %%rdx, %%rax \n\t"
#endif
        "cmp %%ecx, %%ebx \n\t"
        "jl LOOP \n\t"
        "movl %%ebx, %0"
        : "=r"(counter)
        : "r"(buf1), "r"(len)
        : "%eax",  "%ecx"
        );
        return counter;
}

