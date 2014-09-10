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

extern int *id_array, num_cores, num_sockets, *socket, num_events;
extern uint64_t min_event_id;
extern int default_hash_length, max_default_entries;
extern double sqrt_five, max_entry_fraction;
extern int pop_threshold;
extern int bad_rva, global_rva, bad_sample_count, total_function_sample_count;
extern int arch_type_flag, objdump_len, bin_type;
extern char* objdump_bin;
extern uint64_t addr_mask;

typedef void* (*branch_func) (char*) ;
extern branch_func *branch_func_array;
extern void branch_function_init();

#define PERF_READER_VERSION     "0.4"

#define MAX_EVENT_NAME 64

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BUILD_ID_SIZE 20
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#define BITS_PER_LONG __WORDSIZE
#define BITS_PER_BYTE           8
#define BITS_TO_U64(nr)  DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(uint64_t))
#define BITS_TO_LONG(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(unsigned long))
#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONG(bits)]


typedef struct mm_struc * mm_struc_ptr;
typedef struct mm_struc{
                uint32_t pid, tid;
                uint64_t addr;
                uint64_t len;
                uint64_t pgoff;
        } mm_data;



typedef struct perf_file_section {
	uint64_t offset;
	uint64_t size;
}perf_file_section_data;

enum {
        HEADER_RESERVED   = 0,
        HEADER_TRACE_INFO = 1,
        HEADER_BUILD_ID,

        HEADER_HOSTNAME,
        HEADER_OSRELEASE,
        HEADER_VERSION,
        HEADER_ARCH,
        HEADER_NRCPUS,
        HEADER_CPUDESC,
        HEADER_CPUID,
        HEADER_TOTAL_MEM,
        HEADER_CMDLINE,
        HEADER_EVENT_DESC,
        HEADER_CPU_TOPOLOGY,
        HEADER_NUMA_TOPOLOGY,
	HEADER_BRANCH_STACK,
	HEADER_PMU_MAPPINGS,

        HEADER_LAST_FEATURE,
	HEADER_FEAT_BITS	= 256,
} feature_bits_t;

/* pseudo samples injected by perf-inject */
enum perf_user_event_type { /* above any possible kernel type */
        PERF_RECORD_USER_TYPE_START             = 64,
        PERF_RECORD_HEADER_ATTR                 = 64,
        PERF_RECORD_HEADER_EVENT_TYPE           = 65,
        PERF_RECORD_HEADER_TRACING_DATA         = 66,
        PERF_RECORD_HEADER_BUILD_ID             = 67,
        PERF_RECORD_FINISHED_ROUND              = 68,
        PERF_RECORD_HEADER_HOSTNAME             = 69,
        PERF_RECORD_HEADER_OSRELEASE            = 70,
        PERF_RECORD_HEADER_VERSION              = 71,
        PERF_RECORD_HEADER_ARCH                 = 72,
        PERF_RECORD_HEADER_NRCPUS               = 73,
        PERF_RECORD_HEADER_CPUDESC              = 74,
        PERF_RECORD_HEADER_CPUID                = 75,
        PERF_RECORD_HEADER_TOTAL_MEM            = 76,
        PERF_RECORD_HEADER_CMDLINE              = 77,
        PERF_RECORD_HEADER_EVENT_DESC           = 78,
        PERF_RECORD_HEADER_CPU_TOPOLOGY         = 79,
        PERF_RECORD_HEADER_NUMA_TOPOLOGY        = 80,
	PERF_RECORD_HEADER_PMU_MAPPINGS		= 81,
        PERF_RECORD_HEADER_MAX
};

typedef struct perf_file_header {
	uint64_t			magic;
	uint64_t			size;
	uint64_t			attr_size;
	struct perf_file_section	attrs;
	struct perf_file_section	data;
	struct perf_file_section	event_types;
	DECLARE_BITMAP(adds_features, HEADER_FEAT_BITS);
}perf_file_header_data;

struct perf_pipe_file_header {
        uint64_t magic;
        uint64_t size;
};

typedef struct perf_file_attr * perf_file_attr_ptr;

typedef struct perf_file_attr {
        struct perf_event_attr  	attr; /* from perf_event.h */
        struct perf_file_section        ids;
}perf_file_attr_data;

struct attr_event {
        struct perf_event_header header;
        struct perf_event_attr attr;
        uint64_t id[];
};

/*
 * the *_type structs represents event type stripped
 * form their perf_event_header struct
 */
struct perf_trace_event_type {
        uint64_t event_id;
        char    name[MAX_EVENT_NAME];
};

struct build_id_event_type {
        pid_t                    pid;
        uint8_t                  build_id[ALIGN(BUILD_ID_SIZE, sizeof(uint64_t))];
        char                     filename[];
};

struct event_type_event {
        struct perf_event_header header;
        struct perf_trace_event_type event_type;
};

struct tracing_data_event {
        struct perf_event_header header;
        uint32_t size;
};

struct feature_event {
        struct perf_event_header header;
        char data[];
};

struct sdesc {
        uint64_t pos;  /* absolute section start offset */
        uint64_t end;  /* absolute section end offset */
};

typedef struct {
        void *buf;
        struct sdesc cur;
        struct sdesc data;
        struct sdesc feat[HEADER_LAST_FEATURE];
	uint64_t sample_type; /* XXX: assume all events have the same sample_type (perf report does the same) */
	int sample_id_all;  /* true if sample_id_all used by at least one (cannot handle when not all events set this) */
	size_t sz_sample_id_all;
	int fd;
	int needs_bswap; /* needs byte swapping for endianess */
} bufdesc_t;

typedef struct event_id * event_id_ptr;

typedef struct event_attr * event_attr_ptr;

typedef struct event_attr {
	struct perf_event_attr  attr; /* from perf_event.h */
	char *                  name;
	uint64_t *              ids;
	int                     nr_ids;
	} event_attr_data;

typedef struct event_id {
	uint64_t id;	/* unique event identifier */
	int	attr_id;/* index of corresponding attr in attrs */
} event_id_t;

struct build_id_event {
        struct perf_event_header header;
        pid_t                    pid;
        unsigned char            build_id[ALIGN(BUILD_ID_SIZE, sizeof(uint64_t))];
        char                     filename[];
};

/*
 * HEADER_HOSTNAME, HEADER_OSRELEASE
 */
struct str_desc {
        int len;
        char str[];
};

typedef struct index_data_struc{
	int     val;
	int     index;
	}index_data;

struct perf_branch_entry {
       uint64_t from;
       uint64_t to;
       struct {
               uint64_t  mispredicted:1,       /* target mispredicted */
                            predicted:1,       /* target predicted */
                            reserved:62;
       };
};

struct perf_branch_stack {
       uint64_t nr;
       struct perf_branch_entry entries[0];
};


extern event_attr_ptr global_attrs;
extern event_id_ptr global_event_ids;
extern int asm_cutoff, func_cutoff, source_cutoff, max_bb, max_branch;
extern int num_branch, num_sub_branch, num_derived;
extern int source_index, target_index, bb_exec_index, sw_inst_retired_index, next_taken_index;
extern int source_column, target_column, bb_exec_column, sw_inst_retired_column, next_taken_column;
extern int rs_empty_duration_index, call_index, mispredict_index, indirect_index;
extern int rs_empty_duration_index, call_column, mispredict_column, indirect_column;
extern char **fixed_name_list, **branch_name_list;
extern int * fixed_event_index;
extern int column_flag;
extern int family, model;
extern char *arch, *machine, *cpu_desc;
extern uint64_t base_kern_address;
extern uint32_t pid_ker;
extern int aggregate_func_list;
extern char *subst_path_prefix[2];
extern uint64_t * core_start_time, * core_last_time;

mmap_struc_ptr insert_mmap (mm_struc_ptr this_mm, char* filename, uint64_t this_time);
void* insert_event_descriptions(int nr_attrs, int nr_ids, perf_file_attr_ptr attrs, event_id_ptr event_ids);
mmap_struc_ptr find_mmap(mmap_struc_ptr pid_mmap_stack, mm_struc_ptr this_mm, char* filename, uint64_t new_time);
mmap_struc_ptr bind_sample(uint32_t pid, uint64_t ip, uint64_t this_time);
thread_struc_ptr find_thread_struc(process_struc_ptr this_process, uint32_t tid);
process_struc_ptr find_process_struc(uint32_t pid);
process_struc_ptr find_principal_process(mmap_struc_ptr this_mmap);
module_struc_ptr find_module_struc(process_struc_ptr this_process, mmap_struc_ptr this_mmap);
module_struc_ptr bind_mmap(mmap_struc_ptr this_mmap);
mmap_struc_ptr bind_sample(uint32_t pid, uint64_t ip, uint64_t this_time);
process_struc_ptr insert_comm(comm_struc_ptr local_comm);
process_struc_ptr insert_fork(fork_struc_ptr f);
int     increment_module_struc(uint32_t pid, uint32_t tid, uint64_t ip, int this_event, int this_cpu, mmap_struc_ptr this_mmap, uint64_t time_enabled, uint64_t time_running);
void  hotspot_function(pointer_data * global_func_list);
void  src_trg_func_list(pointer_data * global_func_list);
void  hotspot_call_graph(pointer_data * global_func_list);
void * hot_func_asm(pointer_data * global_func_list);
void * hot_list(pointer_data * global_func_list);
int func_asm(pointer_data * global_func_list, int index);
void create_dir();
void multiplex_correction();
void quickSortIndex(index_data *arr, int elements);
int increment_return(mmap_struc_ptr this_mmap, uint64_t source, uint64_t destination, mmap_struc_ptr target_mmap);
int increment_call_site(mmap_struc_ptr this_mmap, uint64_t source, uint64_t destination, mmap_struc_ptr target_mmap);
int increment_next_taken_site(mmap_struc_ptr this_mmap, uint64_t source, uint64_t next_branch, mmap_struc_ptr next_taken_mmap);
uint64_t parse_elf_header(int fd);

