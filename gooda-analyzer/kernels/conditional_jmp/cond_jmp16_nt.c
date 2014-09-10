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

#include <stdio.h> 
#include <strings.h>
#include <fcntl.h>
#include <syscall.h>

static int entries = 0;
static int depth_max = 0;

#define PERM 666

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
	fprintf (fp_main,"\t\"xorq   %%rdx, %%rdx\\n\\t\"\n");
	fprintf (fp_main,"\t\"xorq   %%rcx, %%rcx\\n\\t\"\n");
	fprintf (fp_main,"\t\"inc   %%rcx\\n\\t\"\n");
	fprintf (fp_main,"\t\".align 16\\n\\t\"\n");

        for(j=0;j<max_depth;j++){
		fprintf (fp_main,"\t\"cmp   %%rdx, %%rcx\\n\\t\"\n");
		fprintf (fp_main,"\t\"je LP_%06d\\n\\t\"\n",j);
		fprintf (fp_main,"\t\"xorq   %%rdx, %%rcx\\n\\t\"\n");
		fprintf (fp_main,"\t\".align 16\\n\\t\"\n");
		fprintf (fp_main,"\t\"LP_%06d:\\n\\t\"\n",j);
                }
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
	int max_so,max_chain,max_depth, rval;
	if(argc < 2){
		printf("generator needs one input number, argc = %d\n",argc);
		exit(1);
		}
	max_depth = atoi(argv[1]);
	printf ("  max_depth= %d\n",max_depth);
	rval = create_main(max_depth);
}
