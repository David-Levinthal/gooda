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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"

typedef struct properties * properties_ptr;

typedef struct properties{
	char*	arch;
	char*	cpu_desc;
	char*	path;
	int	family;
	int	model;
	int	num_col;
	int 	num_cores;
	int	num_sockets;
	int	num_fixed;
	int	num_ordered;
	int	num_branch;
	int	num_sub_branch;
	} properties_data;
	
typedef struct csv_field_pointer * csv_field_pointer_ptr;

typedef struct csv_field_pointer{
	csv_field_pointer_ptr	next;
	csv_field_pointer_ptr	previous;
	char *			csv_field;
	}csv_field_pointer_data;


char prop_str[] = "platform_properties.txt";
char func_str[] = "function_hotspots.csv";
char proc_str[] = "process.csv";
char spreadsheets[] = "/spreadsheets/";
int prop_len=23,func_len=21, proc_len=11, spread_len=14;

int dbg_count = 0;

properties_data * input_properties;
event_order_struc_ptr *input_order, global_event_order;

char*
extract_input_string(char* instr, char key, int index, int* next)
{
	int i,j,k,len;
	char* ret_string;

	len = strlen(instr);
	i = 0;
	while((instr[i] != key) && (i <= len))i++;
	if(i == len)
		err(1,"unrecognized input string in read_platform_properties for index = %d, string = %s",index,instr);
	j=i+1;
	while((instr[j] != '\n') && (instr[j] != ',') && (j <= len))j++;
	*next = j+1;
	len = j - i - 1;
	ret_string = malloc(len+1);
	if(ret_string == NULL)
		err(1,"extract_input_string failled to malloc return string of length %d for processing %s",len,instr);
	strncpy(ret_string,&instr[i+1],len);
	ret_string[len]='\0';
//	fprintf(stderr," address of instr = %p, i = %d, j = %d, return_string = %s\n",instr,i,j,ret_string);
	return ret_string;
}
char*
extract_csv_field(char* instr, int index, int* next)
{
	int i,j,k,len;
	char* ret_string;

	len = strlen(instr);
	i = 0;
	while((instr[i] != ',') && (instr[i] != '\n') && (i <= len))
		{
		if(instr[i] == '"')
			{
			i++;
			while(instr[i] != '"')i++;
			}
		i++;
		}
	if(i >= len)
		err(1,"unrecognized input string in read_platform_properties for index = %d, string = %s",index,instr);
	len = i;
	*next = i+1;
	if((i == 0) && (instr[i] == '\n'))len++;
	if((i == 1) && (instr[i] == '\n'))len++;
	ret_string = malloc(len+1);
	if(ret_string == NULL)
		err(1,"extract_input_string failled to malloc return string of length %d for processing %s",len,instr);
	strncpy(ret_string,instr,len);
	ret_string[len]='\0';
	if(dbg_count < 100)
		{
		fprintf(stderr," address of instr = %p, i = %d, return_string = %s\n",instr,i,ret_string);
		dbg_count++;
		}
	return ret_string;
}

char*
read_file(char* local_path)
{
	FILE *fdesc;
	int access_status;
	char* input_str;
	long file_size, test;

	access_status = access(local_path, R_OK);
	if(access_status != 0)
		err(1,"cannot find the platform_properties.txt file from path %s",local_path);
	fdesc = fopen(local_path, "r");
	if(fdesc == NULL)
		err(1, "failed to open file %s\n",local_path);
	fseek(fdesc, 0, SEEK_END);
	file_size = ftell(fdesc);
	rewind(fdesc);

	input_str = (char*)malloc(sizeof(char)*file_size);
	if(input_str == NULL)
		err(1," failed to malloc buffer for reading input file %s size = %ld",local_path,file_size);
	test = fread(input_str,1,file_size,fdesc);
	if(test != file_size)
		err(1,"read size, %ld != file_size %ld for file %s",test,file_size,local_path);

	return input_str;
}
int
read_platform_properties(int index)
{

	int i,j,k,len,ret=0, access_status, test;
	char * local_prop, *input_str, *ret_str;
	FILE * pr;
	long file_size;
	int begin_line=0, read_start = 0;;

	len = strlen(input_properties[index].path);
//	fprintf(stderr," index = %d, len = %d, path = %s\n",index,len,input_properties[index].path);
	local_prop = malloc(len + spread_len + prop_len + 1);
	if(local_prop == NULL)
		err(1,"malloc failed for input properties index = %d",index);
	strncpy(local_prop, input_properties[index].path, len);
	local_prop[len] = '\0';
//	fprintf(stderr,"local_prop = %s\n",local_prop);
	strncpy(&local_prop[len], spreadsheets, spread_len);
	local_prop[len+spread_len] = '\0';
//	fprintf(stderr,"local_prop = %s\n",local_prop);
	strncpy(&local_prop[len+spread_len], prop_str, prop_len);

//	for(i=0; i < len; i++)local_prop[i] = input_properties[index].path[i];
//	for(i=len; i < len + spread_len; i++)local_prop[i] = spreadsheets[i-len];
//	for(i=len + spread_len; i < len + spread_len + prop_len; i++)local_prop[i] = prop_str[i-len-spread_len];

	local_prop[len + spread_len + prop_len] = '\0';
	fprintf(stderr,"local_prop = %s\n",local_prop);

	input_str = read_file(local_prop);
	if(input_str == NULL)
		err(1,"read_file returned a NULL pointer for index %d path = %s",index,local_prop);

	len = strlen(input_str);
	fprintf(stderr," length of input_str = %d\n",len);
//	fprintf(stderr,"input_str = \n");
//	fprintf(stderr,"%s\n",input_str);
	input_properties[index].arch = extract_input_string(input_str, ':', index, &begin_line);
	read_start+=begin_line;
	input_properties[index].family = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].model = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_sockets = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_cores = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_col = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].cpu_desc = extract_input_string(&input_str[read_start], ':', index, &begin_line);
	read_start+=begin_line;
	input_properties[index].num_fixed = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_ordered = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_branch = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	read_start+=begin_line;
	input_properties[index].num_sub_branch = atoi(extract_input_string(&input_str[read_start], ':', index, &begin_line));
	fprintf(stderr,"arch = %s\n",input_properties[index].arch);
	fprintf(stderr,"cpu_desc = %s\n",input_properties[index].cpu_desc);
	fprintf(stderr,"family = %d, model = %d\n",input_properties[index].family,input_properties[index].model);
	fprintf(stderr,"num_sockets = %d, num_cores = %d\n",input_properties[index].num_sockets,input_properties[index].num_cores);
	fprintf(stderr,"num_col = %d\n",input_properties[index].num_col);
	fprintf(stderr,"num_fixed = %d, num_ordered = %d\n",input_properties[index].num_fixed,input_properties[index].num_ordered);
	fprintf(stderr,"num_branch = %d, num_sub_branch = %d\n",input_properties[index].num_branch,input_properties[index].num_sub_branch);
	return 0;
}

int
read_func_table(int index)
{
	int i,j,k,len,ret=0, field_len, fields_per_line=0, lines=0;;
	char * local_func, *input_str, *ret_str;
	FILE *func_tab;
	long file_size;
	int begin_line=0, read_start=0;

	len = strlen(input_properties[index].path);
//	fprintf(stderr," index = %d, len = %d, path = %s\n",index,len,input_properties[index].path);
	local_func = malloc(len + spread_len + func_len + 1);
	if(local_func == NULL)
		err(1,"malloc failed for input properties index = %d",index);
	strncpy(local_func, input_properties[index].path, len);
	local_func[len] = '\0';
//	fprintf(stderr,"local_func = %s\n",local_func);
	strncpy(&local_func[len], spreadsheets, spread_len);
	local_func[len+spread_len] = '\0';
//	fprintf(stderr,"local_func = %s\n",local_func);
	strncpy(&local_func[len+spread_len], func_str, func_len);

	local_func[len + spread_len + func_len] = '\0';
	fprintf(stderr,"local_func = %s\n",local_func);

	input_str = read_file(local_func);
	if(input_str == NULL)
		err(1,"read_file returned a NULL pointer for index %d path = %s",index,local_func);

	len = strlen(input_str);
	fprintf(stderr," length of input_str = %d\n",len);

	while(read_start < len)
		{
		ret_str = extract_csv_field(&input_str[read_start],index,&begin_line);
		fields_per_line++;
//		field_len = strlen(ret_str);
		field_len = begin_line-1;
		if(ret_str[field_len] == '\n')
			{
			lines++;
			fprintf(stderr,"line number %d, fields_per_line = %d\n",lines,fields_per_line);
			fields_per_line = 0;
			}
		read_start+=begin_line;
		}
	return 0;
}

int read_proc_table(int index)
{
	int i,j,k,len,ret=0;
	char * local_func, *local_proc;

	return 0;
}
void
usage()
{
	fprintf(stderr,"multi_perf creates a new ./spreadsheets tree by summing the table contents\n");
	fprintf(stderr,"from the spreadsheet directories in a list of input report directory paths or\n");
	fprintf(stderr,"by taking the difference of the tables in the spreadsheet directories of 2 report paths\n");
	fprintf(stderr,"The first argument must be -n followed by the number of report directories to come (with or without a space)\n");
	fprintf(stderr," each report directory path (absolute or relative) is input with the -d flag, one directory for each -d\n");
	fprintf(stderr," -a (with no argument value) indicates the tables should be summed\n");
	fprintf(stderr," -s (with no argument value) indicates a subtraction of the table data and the value for -n MUST be 2\n");
	fprintf(stderr," -h produces this output\n");
}

int
main( int argc, char** argv)
{

	int i,j,k,dir=0,len,len1,ret;
	int num_dir=0, add = 0, sub = 0;
	int c;
	char* path_buf;

	fprintf(stderr,"argc = %d\n",argc);
	for(i=0;i<argc; i++)fprintf(stderr,"argv[%d] = %s\n",i,argv[i]);

	while ((c= getopt(argc, argv, "h:n:d:a:s")) != -1) {
                switch(c) {
                case 'h':
                        usage();
                        exit(0);
		case 'n':
			num_dir = atoi(optarg);
			fprintf(stderr," num_dir = %d, sizeof(properties_data) = %ld\n",num_dir, sizeof(properties_data));
//			malloc array of structures for the platform properites data
			input_properties = (properties_data*)calloc(1,(num_dir+1)*sizeof(properties_data));
			fprintf(stderr,"finished calloc for properties_data at %p\n",input_properties);
			if(input_properties == NULL)
				err(1," failed to malloc buffer for input_properties array");
			break;
                case 'd':
			fprintf(stderr,"argument = -d\n");
			if(num_dir == 0){
				usage();
				err(1,"the number of directories argument -n must appear before any of the directory arguments, -d");
				}
			fprintf(stderr,"address of optarg = %p\n",optarg);
			fprintf(stderr," -d arg = %s\n",optarg);
			len = strlen(optarg);
			if(dir < num_dir){
				dir++;
//				fprintf(stderr,"address of *path = %p\n",&input_properties[dir].path);
//				for(i=0;i<=num_dir;i++)fprintf(stderr," loop index %d, address of *path = %p\n",i,&input_properties[i].path);
//				path_buf = (char*)malloc(len+1);
//				if(path_buf == NULL)
//					err(1,"cannot malloc buffer for path %d, len = %d",dir,len);
//				for(i=0; i<len; i++)path_buf[i] = optarg[i];
//				path_buf[len] = '\0';
//				fprintf(stderr,"address of path_buf = %p, path_buf = %s\n",path_buf,path_buf);
				input_properties[dir].path = (char*)malloc(len+1);
				if(input_properties[dir].path == NULL)
					err(1,"cannot malloc buffer for path %d, len = %d",dir,len);
				for(i=0; i<len; i++)input_properties[dir].path[i] = optarg[i];
				input_properties[dir].path[len] = '\0';

//				input_properties[dir].path = path_buf;
				len1 = strlen(input_properties[dir].path);
//				fprintf(stderr," dir = %d, len = %d, len1 = %d, path = %s\n",dir,len,len1,input_properties[dir].path);
//				fprintf(stderr,"address of path = %p, = %p\n",&input_properties[dir].path,&input_properties[dir].path[0]);
//				for(i=1;i<=dir;i++){
//				len1 = strlen(input_properties[i].path);
//				fprintf(stderr," dir = %d, len1 = %d, path = %s,  address = %p\n",i,len1,input_properties[i].path,input_properties[i].path);
//				}
				break;
				}
			else{
				err(1," more directories listed with -d flag than were declared with -n");
				}
		case 'a':
			fprintf(stderr," -a arg, num_dir = %d, sub = %d\n",num_dir,sub);
			if((sub != 0) || (num_dir < 2))
				err(1," cannot set both -a and -s or value of -n argument less than 2");
			add = 1;
			break;
		case 's':
			if((add != 0) || (num_dir < 2))
				err(1," cannot set both -a and -s or value of -n argument less than 2");
			sub = 1;
			break;
		default:
			errx(1, "invalid argument key");
		}
	}
	fprintf(stderr,"exited input arg case statement\n");
//	create_dir();

	for(i=1; i<=num_dir; i++)
		{
		len1 = strlen(input_properties[i].path);
		fprintf(stderr," i = %d, len1 = %d, path = %s\n",i,len1,input_properties[i].path);
		ret = read_platform_properties(i);
		if(ret != 0)
			err(1,"could not read platform properties for path %d = %s",i,input_properties[i].path);
		ret = read_func_table(i);
		if(ret != 0)
			err(1,"could not read function table for path %d = %s",i,input_properties[i].path);
		}

}
