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

//#include "asm_2_src.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include "bfd.h"

static const char *filename;
static const char *functionname;
static unsigned int line;
static asymbol **syms;
static bfd_vma ip;
static bfd_boolean found;
static bfd *abfd, *dbg_bfd = NULL, *active_bfd;
char rel_path[] = "./debug";
int rel_len=7;

static void
locate_function(bfd *self, asection *section,void *data ATTRIBUTE_UNUSED)
{

	bfd_vma vma;
	bfd_size_type size;

	if(found)
		return;

	if((bfd_get_section_flags (active_bfd, section) & SEC_ALLOC) == 0)
		return;

	vma = bfd_get_section_vma (active_bfd, section);
	if(ip < vma)
		return;

	size = bfd_get_section_size (section);
	if(ip >= vma + size)
		return;

	found = bfd_find_nearest_line (self, section, syms, ip - vma,
		&filename, &functionname, &line);
}
static int
 has_debug_line_info(bfd *abfd)
{
	return (bfd_get_section_by_name (abfd, ".debug_line") != NULL ||
		bfd_get_section_by_name(abfd, ".zdebug_line") != NULL);
}

static int 
process_symtab(void)
{
	long storage;
	long symcount;
	const char *string, *errmsg;
	int debug_line_info;

	bfd_boolean dynamic = FALSE;

	debug_line_info = has_debug_line_info(active_bfd);
#ifdef DBUG
	fprintf(stderr," from process_symtab/asm_2_src_init, debug_line_info = %d\n",debug_line_info);
#endif

	if ((bfd_get_file_flags (active_bfd) & HAS_SYMS) == 0)
		{
		string = bfd_get_filename (active_bfd);
		errmsg = bfd_errmsg(bfd_get_error());
		fflush(stdout);
		if(string)
			fprintf(stderr, "%s: %s\n",string, errmsg);
		else
			fprintf(stderr, "%s\n", errmsg);

#ifdef DBUG
		fprintf(stderr,"from process_symtab returning -1\n");
#endif
		return -1;
		}

	storage = bfd_get_symtab_upper_bound (active_bfd);
	if (storage == 0) {
		storage = bfd_get_dynamic_symtab_upper_bound (active_bfd);
		dynamic = TRUE;
	}
	if (storage < 0)
		{
		string = bfd_get_filename (active_bfd);
		errmsg = bfd_errmsg(bfd_get_error());
		fflush(stdout);
		if(string)
			fprintf(stderr, "%s: %s\n",string, errmsg);
		else
			fprintf(stderr, "%s\n", errmsg);

#ifdef DBUG
		fprintf(stderr,"from process_symtab returning -1\n");
#endif
		return -1;
		}

	syms = (asymbol **) malloc(storage);
	if (dynamic)
		symcount = bfd_canonicalize_dynamic_symtab (active_bfd, syms);
	else
		symcount = bfd_canonicalize_symtab (active_bfd, syms);
	if (symcount < 0)
		{
		string = bfd_get_filename (active_bfd);
		errmsg = bfd_errmsg(bfd_get_error());
		fflush(stdout);
		if(string)
			fprintf(stderr, "%s: %s\n",string, errmsg);
		else
			fprintf(stderr, "%s\n", errmsg);

		fprintf(stderr,"from process_symtab returning -1\n");
		return -1;
		}
#ifdef DBUG
	fprintf(stderr,"from process_symtab returning value based on debug_line_info\n");
#endif
	if(debug_line_info == 1)return 0;
	return -1;
}

int 
asm_2_src_init(const char *file_name)
{
	const char *string, *errmsg;
	bfd_size_type this_size;
	char* dbg_filename, *full_dbg_filename;
	int access_status,i;

	asection *this_section;

	abfd = bfd_openr(file_name, NULL);
	if (abfd == NULL)
		{
		string = file_name;
		errmsg = bfd_errmsg(bfd_get_error());
		fflush(stdout);
		if(string)
			fprintf(stderr, "%s: %s\n",string, errmsg);
		else
			fprintf(stderr, "%s\n", errmsg);

		return -1;
		}

	if (!bfd_check_format(abfd, bfd_object))
		{
		string = bfd_get_filename (abfd);
		errmsg = bfd_errmsg(bfd_get_error());
		fflush(stdout);
		if(string)
			fprintf(stderr, "%s: %s\n",string, errmsg);
		else
			fprintf(stderr, "%s\n", errmsg);

		return -1;
		}


	active_bfd = abfd;
	dbg_bfd = NULL;
//	check for external debug file
	if((this_section = bfd_get_section_by_name(abfd, ".gnu_debuglink")) != NULL)
		{
		this_size = bfd_section_size(abfd, this_section);
		dbg_filename = malloc(this_size);
		if(dbg_filename == NULL)
			err(1,"malloc failed for dbg_filename in asm_2_src_init");
		full_dbg_filename = malloc(this_size + rel_len+1);
		if(full_dbg_filename == NULL)
			err(1,"malloc failed for full_dbg_filename in asm_2_src_init");

		bfd_get_section_contents(abfd, this_section, dbg_filename, (file_ptr)0, (bfd_size_type)this_size);

		for(i=0; i<rel_len; i++)full_dbg_filename[i] = rel_path[i];
		for(i=rel_len; i<rel_len + this_size; i++)full_dbg_filename[i] = dbg_filename[i-rel_len];
		full_dbg_filename[rel_len+this_size] = '\0';

//		is the file there?
		access_status = access(full_dbg_filename, R_OK);
		if( access_status == 0)
			{
//			we have a debug info file, and even found in in ./debug
			if(syms != NULL)free(syms);
			syms = NULL;
			bfd_close(abfd);
			abfd = NULL;

			dbg_bfd = bfd_openr((const char *)full_dbg_filename, NULL);
		        if (dbg_bfd == NULL)
		                {
		                string = file_name;
		                errmsg = bfd_errmsg(bfd_get_error());
		                fflush(stdout);
		                if(string)
		                        fprintf(stderr, "%s: %s\n",string, errmsg);
		                else
		                        fprintf(stderr, "%s\n", errmsg);

		                return -1;
		                }

		        if (!bfd_check_format(dbg_bfd, bfd_object))
		                {
		                string = bfd_get_filename (dbg_bfd);
		                errmsg = bfd_errmsg(bfd_get_error());
		                fflush(stdout);
		                if(string)
		                        fprintf(stderr, "%s: %s\n",string, errmsg);
		                else
		                        fprintf(stderr, "%s\n", errmsg);

		                return -1;
		                }
			active_bfd = dbg_bfd;
			}
		}
#ifdef DBUG
	fprintf(stderr," from process_symtab filename = %s\n",file_name);
#endif
	return process_symtab();
}

void 
asm_2_src_close(void)
{
	if (syms != NULL) {
		free(syms);
		syms = NULL;
	}

	if(abfd != NULL)bfd_close(abfd);
	if(dbg_bfd != NULL)bfd_close(dbg_bfd);
	line = found = 0;
}

int 
asm_2_src_inline(const char **file, unsigned *line_nr)
{

	found = bfd_find_inliner_info (active_bfd, &filename, &functionname, &line);
	*file = filename;
	*line_nr = line;

	return found;
}

int asm_2_src(unsigned long addr, const char **file, unsigned *line_nr)
{

	found = 0;
	ip = addr;
	bfd_map_over_sections(active_bfd, locate_function, NULL);

	*file = filename;
	*line_nr = line;

	return found;
}
