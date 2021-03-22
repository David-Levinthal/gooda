/*
Copyright 201r42 Google Inc. All Rights Reserved.

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

#include <stdio.h> 
#include <strings.h>
#include <fcntl.h>
#include <syscall.h>
#include <stdlib.h>

double drand48(void);


static int entries = 0;
static int depth_max = 0;

#define PERM 666


int *ran_array, *ordered_ran_array, *ordered_target;
int current=0,depth=0;
int max_xor;

#define paste(front, back) front ## back

static int create_main(int max_depth)
{
	int i,j,k,rval;
	FILE  * fp_main;
	char filename[100];
	fp_main = fopen("FOO_main.c","w+");

	fprintf (fp_main,"//\n");
	fprintf (fp_main,"//\n");
	fprintf (fp_main,"#include <stdio.h>\n");
	fprintf (fp_main,"\n");
	fprintf (fp_main,"int main(int argc, char* argv[])\n");
	fprintf (fp_main,"{\n");
	fprintf (fp_main,"\tlong i=0;\n");
	fprintf (fp_main,"\tlong j,k;\n");
	fprintf (fp_main,"\tj=atol(argv[1]);\n");

	fprintf (fp_main,"\tfor(k=0;k<j;k++){\n");

        fprintf (fp_main,"\t__asm__( \n");
	fprintf (fp_main,"\t\".align 16\\n\\t\"\n");
	fprintf (fp_main,"\t\"jmp LP_%06d\\n\\t\"\n",ran_array[0]);

        for(j=0;j<max_depth;j++){
		fprintf (fp_main,"\t\".align 16\\n\\t\"\n");
		fprintf (fp_main,"\t\"LP_%06d:\\n\\t\"\n",ordered_ran_array[j]);
		fprintf (fp_main,"\t\"jmp LP_%06d\\n\\t\"\n",ran_array[ordered_target[j]+1]);
		fprintf (fp_main,"\t\"xorq   %%rdx, %%rcx\\n\\t\"\n");
                }
	fprintf (fp_main,"\t\"LP_%06d:\\n\\t\"\n",ran_array[max_depth]);
        fprintf (fp_main,"\t);\n");

	fprintf (fp_main,"\t}\n");
	fprintf (fp_main,"\n");
	fprintf (fp_main,"\treturn 0;\n");
	fprintf (fp_main,"}\n");


	rval=fclose(fp_main);

	return 0;
}

main(int argc, char* argv[])
{
	int max_depth, rval, i, ranval, *temp_array, n=0, max_ran=1000000;
	if(argc < 2){
		printf("generator needs one input number, argc = %d\n",argc);
		exit(1);
		}
	max_depth = atoi(argv[1]);
	printf ("  max_depth= %d\n",max_depth);
//	the mechanics here causes one extra final jump to be generated to the terminating label
//      reduce max_depth by 1
	max_depth--;
        if(max_depth >= max_ran){
                fprintf(stderr,"too many functions %d, must be less than %d\n",max_depth,max_ran);
                exit(1);
                }
        ran_array = (int*) malloc((max_depth+1)*sizeof(int));
        if(ran_array == 0){
                fprintf(stderr,"failed to malloc random array\n");
                exit(1);
                }
        ordered_ran_array = (int*) malloc(max_depth*sizeof(int));
        if(ordered_ran_array == 0){
                fprintf(stderr,"failed to malloc ordered random array\n");
                exit(1);
                }
        ordered_target = (int*) malloc((max_depth+1)*sizeof(int));
        if(ordered_target == 0){
                fprintf(stderr,"failed to malloc ordered target array\n");
                exit(1);
                }

        temp_array = (int*)malloc(10*max_depth*sizeof(int));
        if(temp_array == 0){
                fprintf(stderr,"failed to malloc temp random array\n");
                exit(1);
                }

        for(i=0;i<10*max_depth;i++)temp_array[i]=-1;
//         warm up rand48
        for(i=0;i<100;i++)ranval+=drand48();
        for(i=0;i<max_depth;i++){
                ranval =(int)( (double)(max_depth)*10.*drand48());
                if((ranval < 0) || (ranval >= 10*max_depth)){
                        fprintf(stderr," bad ranval = %d, max_depth = %d\n",ranval,max_depth);
                        exit(1);
                        }
                while(temp_array[ranval] >= 0){
                        ranval++;
                        if(ranval >= 10*max_depth)ranval=0;
                        }
                ran_array[i]=ranval;
                temp_array[ranval] = i;
                }
	n=0;
	for(i=0;i<10*max_depth;i++){
		if(temp_array[i] >= 0){
			ordered_ran_array[n]=i;
			ordered_target[n]=temp_array[i];
			n++;
			if(n>max_depth){
				fprintf(stderr,"too many values in temp_array, should be less than %d, we have %d\n",max_depth,n);
				exit(1);
				}
			}
		}
	if(n != max_depth){
		fprintf(stderr,"should have found %d random numbers generated but only found %d\n",max_depth-1,n);
		exit(1);
		}
	ran_array[max_depth] = max_ran;
	ordered_target[max_depth] = max_depth;
	rval = create_main(max_depth);
}
