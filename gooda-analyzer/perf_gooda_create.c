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

//      structure creation for Gooda
//     Generic Optimization Data Analyzer
//     Dispencer of wisdom and Insight
//     aka   DWI


#include <sys/types.h>
#include <stdio.h>
#include <err.h>
#include <malloc.h>

#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"

mmap_struc_ptr 
mmap_struc_create(void)
{
        mmap_struc_ptr this_struc;
/*
        this_struc = (mmap_struc_ptr) malloc(sizeof(mmap_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->filename = NULL;
        this_struc->addr = 0;
        this_struc->len = 0;
        this_struc->time = 0;
        this_struc->this_module = NULL;
        this_struc->this_process = NULL;
        this_struc->principal_process = NULL;
        this_struc->pid = 0;
        this_struc->tid = 0;
        this_struc->pgoff = 0;
        this_struc->tsc_start = 0;
        this_struc->tsc_end = 0;
        this_struc->last_pgoff = 0;
*/
	this_struc = calloc(1,sizeof(mmap_data));
        return this_struc;
}

lost_struc_ptr 
lost_struc_create(void)
{
        lost_struc_ptr this_struc;
/*
        this_struc = (lost_struc_ptr) malloc(sizeof(lost_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->time = 0;
        this_struc->id = 0;
        this_struc->lost = 0;
*/
	this_struc = calloc(1,sizeof(lost_data));
        return this_struc;
}

comm_struc_ptr 
comm_struc_create(void)
{
        comm_struc_ptr this_struc;
/*
        this_struc = (comm_struc_ptr) malloc(sizeof(comm_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
	this_struc->first_mmap = NULL;
        this_struc->name = NULL;
        this_struc->time = 0;
        this_struc->pid = 0;
        this_struc->tid = 0;
*/
	this_struc = calloc(1,sizeof(comm_data));
        return this_struc;
}

exit_struc_ptr 
exit_struc_create(void)
{
        exit_struc_ptr this_struc;
/*
        this_struc = (exit_struc_ptr) malloc(sizeof(exit_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->time = 0;
        this_struc->pid = 0;
        this_struc->ppid = 0;
        this_struc->tid = 0;
        this_struc->ptid = 0;
*/
	this_struc = calloc(1,sizeof(exit_data));
        return this_struc;
}

throttle_struc_ptr 
throttle_struc_create(void)
{
        throttle_struc_ptr this_struc;
/*
        this_struc = (throttle_struc_ptr) malloc(sizeof(throttle_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->time = 0;
        this_struc->id = 0;
        this_struc->stream_id = 0;
*/
	this_struc = calloc(1,sizeof(throttle_data));
        return this_struc;
}

unthrottle_struc_ptr 
unthrottle_struc_create(void)
{
        unthrottle_struc_ptr this_struc;
/*
        this_struc = (unthrottle_struc_ptr) malloc(sizeof(unthrottle_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->time = 0;
        this_struc->id = 0;
        this_struc->stream_id = 0;
*/
	this_struc = calloc(1,sizeof(unthrottle_data));
        return this_struc;
}

fork_struc_ptr 
fork_struc_create(void)
{
        fork_struc_ptr this_struc;
/*
        this_struc = (fork_struc_ptr) malloc(sizeof(fork_data));
        if(this_struc == NULL)return this_struc;
        this_struc->pid = 0;
        this_struc->ppid = 0;
        this_struc->tid = 0;
        this_struc->ptid = 0;
        this_struc->time = 0;
*/
	this_struc = calloc(1,sizeof(fork_data));
        return this_struc;
}

read_struc_ptr 
read_struc_create(void)
{
        read_struc_ptr this_struc;
/*
        this_struc = (read_struc_ptr) malloc(sizeof(read_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->buffer = NULL;
        this_struc->time = 0;
        this_struc->size = 0;
*/
	this_struc = calloc(1,sizeof(read_data));
        return this_struc;
}

raw_sample_struc_ptr 
raw_sample_struc_create(void)
{
        raw_sample_struc_ptr this_struc;
/*
        this_struc = (raw_sample_struc_ptr) malloc(sizeof(raw_sample_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
	this_struc->time = 0;
	this_struc->ip = 0;
	this_struc->pebs_ip = 0;
	this_struc->str_id = 0;
	this_struc->sample_period = 0;
	this_struc->pid = 0;
	this_struc->tid = 0;
	this_struc->cpu = 0;
*/
	this_struc = calloc(1,sizeof(raw_sample_data));
        return this_struc;
}

pmu_programming_struc_ptr 
pmu_programming_struc_create(void)
{
        pmu_programming_struc_ptr this_struc;
/*
        this_struc = (pmu_programming_struc_ptr) malloc(sizeof(pmu_programming_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
*/
	this_struc = calloc(1,sizeof(pmu_programming_data));
        return this_struc;
}
