
#include <stdio.h> 
#include <strings.h>
#include <fcntl.h>
#include <syscall.h>
#include <stdlib.h>

static int entries = 0;
static int depth_max = 0;

#define PERM 666

#define paste(front, back) front ## back

static int genC (int star_so, int chain, int depth, FILE* fp, FILE* fp_header)
{
	int j, depth_p, even_p;
	fprintf (fp_header,"extern long FOO_%03d_%03d_%03d (long n);\n", star_so,chain,depth);

	fprintf (fp,"//\n");
	fprintf (fp,"//\n");
	fprintf (fp,"#include <stdio.h>\n");
	fprintf (fp,"#include \"FOO.h\"\n");
	fprintf (fp,"\n");
	fprintf (fp,"long FOO_%03d_%03d_%03d (long n)\n", star_so,chain,depth);
	fprintf (fp,"{\n");
//	fprintf (fp,"\t__asm__( \n");

/*	for(j=0;j<50;j++){
		fprintf (fp,"\t\"xorq    %%rdx, %%rdx\\n\\t\"\n");
		}
*/
//	fprintf (fp,"\t);\n");

	if(depth < depth_max-1) {

			depth_p = depth+1;

		fprintf(fp,"\tFOO_%03d_%03d_%03d( n+1);\n",star_so,chain,depth_p);
		}
	fprintf(fp,"\treturn n;\n");
	fprintf (fp,"}\n");

	return 0;
}

static int create_source(int max_so, int max_chain, int max_depth)
{
	int i,j,k,rval;
	FILE * fp, * fp_header;
	char filename[100];
	fp_header = fopen("FOO.h","w+");

	for(i=0; i<max_so; i++){
	   for(j=0; j<max_chain; j++){	
	      for(k=0; k<max_depth; k++){
		sprintf(filename,"FOO_%03d_%03d_%03d.c",i,j,k);
		fp = fopen(filename,"w+");
		rval =genC(i,j,k,fp, fp_header);
		rval=fclose(fp);
		}
	     }
	   }

	rval=fclose(fp_header);

	return 0;
}
static int create_main(int max_so, int max_chain, int max_depth)
{
	int i,j,k,rval;
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
	fprintf (fp_main,"\tlong i=0;\n");
	fprintf (fp_main,"\tlong j,k,loop;\n");
	fprintf (fp_main,"\tloop=atol(argv[1]);\n");
	fprintf (fp_main,"\tfor(k=0;k<loop;k++){\n");


	for(i=0; i<max_so; i++){
	   for(j=0; j<max_chain; j++){	
		fprintf(fp_main, "\t\tFOO_%03d_%03d_000(i);\n",i,j);
		fprintf (fp_main,"\t\ti+=1000;\n");
		}
	    }
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
	if(argc < 4){
		printf("generator needs three input numbers, argc = %d\n",argc);
		exit(1);
		}
	max_so = atoi(argv[1]);
	max_chain = atoi(argv[2]);
	max_depth = atoi(argv[3]);
	printf (" max_s0 = %03d, max_chain = %03d, max_depth= %03d\n",max_so,max_chain,max_depth);
	depth_max=max_depth;
	rval = create_source(max_so,max_chain,max_depth);
	rval = create_main(max_so,max_chain,max_depth);
}
