
#include <stdio.h> 
#include <strings.h>
#include <fcntl.h>
#include <syscall.h>
#include <stdlib.h>

static int entries = 0;
static int depth_max = 0;

#define PERM 666

#define paste(front, back) front ## back
int chk_full(int* rand_array, int max_blocks);
void get_rand(int* rand_array, int max_blocks);

static int genC (int star_so, int chain, int depth, FILE* fp, FILE* fp_header, int* rand_array)
{
	int j, depth_p, even_p;
	fprintf (fp_header,"extern long FOO_%03d_%03d_%03d (long n, long m);\n", star_so,chain,depth);

	fprintf (fp,"//\n");
	fprintf (fp,"//\n");
	fprintf (fp,"#include <stdio.h>\n");
	fprintf (fp,"#include \"FOO.h\"\n");
	fprintf (fp,"\n");
	fprintf (fp,"long FOO_%03d_%03d_%03d (long n, long m)\n", star_so,chain,depth);
	fprintf (fp,"{\n");
	fprintf (fp,"\tlong max_depth = %d;\n",depth_max);
	fprintf (fp,"\t__asm__( \n");

//	for(j=0;j<50;j++){
//		fprintf (fp,"\t\"xorq    %%rdx, %%rdx\\n\\t\"\n");
//		}
	fprintf (fp,"\t\".align 64\\n\\t\"\n");
	fprintf (fp,"\t);\n");


	depth_p = rand_array[depth];
	fprintf (fp,"\tif((long)(m+1) < max_depth)\n");

	fprintf(fp,"\t\tFOO_%03d_%03d_%03d( n+1, m+1);\n",star_so,chain,depth_p);


	fprintf(fp,"\treturn n;\n");
	fprintf (fp,"}\n");

	return 0;
}

static int create_source(int max_so, int max_chain, int max_depth, int* rand_array)
{
	int i,j,k,rval;
	FILE * fp, * fp_header;
	char filename[100];
	fp_header = fopen("FOO.h","w+");

	for(i=0; i<max_so; i++){
	   for(j=0; j<max_chain; j++){	
	      get_rand(rand_array, max_depth);

	      for(k=0; k<max_depth; k++){
		sprintf(filename,"FOO_%03d_%03d_%03d.c",i,j,k);
		fp = fopen(filename,"w+");
		rval =genC(i,j,k,fp, fp_header, rand_array);
		rval=fclose(fp);
		}
	     }
	   }

	rval=fclose(fp_header);

	return 0;
}
static int create_main(int max_so, int max_chain, int max_depth, int* rand_array)
{
	int i,j,k,rval, start;
	FILE  * fp_main;
	char filename[100];
	fp_main = fopen("FOO_main.c","w+");

	fprintf (fp_main,"//\n");
	fprintf (fp_main,"//\n");
	fprintf (fp_main,"#include <stdio.h>\n");
	fprintf (fp_main,"#include <stdlib.h>\n");
	fprintf (fp_main,"#include \"FOO.h\"\n");
	fprintf (fp_main,"\n");
	fprintf (fp_main,"int main(int argc, char* argv[])\n");
	fprintf (fp_main,"{\n");
	fprintf (fp_main,"\tlong i=0, d=0;\n");
	fprintf (fp_main,"\tlong j,k,loop;\n");
	fprintf (fp_main,"\tloop=atol(argv[1]);\n");
	fprintf (fp_main,"\tfor(k=0;k<loop;k++){\n");


	for(i=0; i<max_so; i++){
	   for(j=0; j<max_chain; j++){
		start = rand_array[(i+j)%max_depth ];	
		fprintf(fp_main, "\t\td = 0;\n");
		fprintf(fp_main, "\t\tFOO_%03d_%03d_%03d(i,d);\n",i,j,start);
		fprintf (fp_main,"\t\ti += 1000;\n");
		}
	    }
	fprintf (fp_main,"\t}\n");
	fprintf (fp_main,"\n");
	fprintf (fp_main,"\treturn 0;\n");
	fprintf (fp_main,"}\n");


	rval=fclose(fp_main);

	return 0;
}
void get_rand(int* rand_array, int max_blocks)
{
	int i, r, idx, val, iter=0;;

	for (i=0; i < max_blocks; i++)
	{
		rand_array[i] = -1;
	}

	idx=0;
	for(;;)
	{
		iter++;
		val = rand();
		for(;;)
		{
			r = rand()%max_blocks;
			if (rand_array[r] == -1)
				break;
		}

		if (rand_array[idx] == -1 && idx != r)
		{
			rand_array[idx] = (int)r;
			idx = r;
		}
/*
		if ((iter%100) == 0)
		{
			Sleep(100);
			printf ("r=%d\tidx=%d\trand[]=%d\n",  r, idx, rand_array[idx]);
		}
*/
		if (chk_full(rand_array, max_blocks) == 1)
		{
			rand_array[idx] = (int)max_blocks;
			rand_array[idx] = 0;  //0 to max_blocks - 1
			break;
		}

	}
/*
	for (i=0; i < max_blocks; i++)
	{
		printf ("%.4d\t%.4d\n", i, rand_array[i]);
		//if ((i%15) ==0) printf("\n");
	}
*/
}

int chk_full(int* rand_array, int max_blocks)
{
	int cnt=0, i;
	for (i=0; i < max_blocks; i++)
	{
		if (rand_array[i] == (int)-1)
		{
			cnt++;
		}
	}
	return cnt;
}

main(int argc, char* argv[])
{
	int max_so,max_chain,max_depth, rval;
	int * rand_array;
	if(argc < 4){
		printf("generator needs three input numbers, argc = %d\n",argc);
		exit(1);
		}
	max_so = atoi(argv[1]);
	max_chain = atoi(argv[2]);
	max_depth = atoi(argv[3]);

	printf (" max_s0 = %03d, max_chain = %03d, max_depth= %03d\n",max_so,max_chain,max_depth);
	depth_max=max_depth;

	rand_array = (int*) malloc( max_depth*sizeof(int));
	get_rand(rand_array, max_depth);

	rval = create_source(max_so,max_chain,max_depth, rand_array);
	get_rand(rand_array, max_depth);
	rval = create_main(max_so,max_chain,max_depth, rand_array);
}
