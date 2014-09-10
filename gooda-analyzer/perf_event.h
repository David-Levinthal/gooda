/*
 * Performance events:
 *
 *
 * Data type definitions, declarations, prototypes.
 *
 */
#ifndef _LINUX_PERF_EVENT_H
#define _LINUX_PERF_EVENT_H

#include <sys/types.h>
#include <inttypes.h>
#ifndef PR_TASK_PERF_EVENTS_DISABLE
#define PR_TASK_PERF_EVENTS_DISABLE           31
#define PR_TASK_PERF_EVENTS_ENABLE            32
#endif

/*
 * hw_event.type
 */
enum perf_type_id {
	PERF_TYPE_HARDWARE		= 0,
	PERF_TYPE_SOFTWARE		= 1,
	PERF_TYPE_TRACEPOINT		= 2,
	PERF_TYPE_HW_CACHE		= 3,
	PERF_TYPE_RAW			= 4,
	PERF_TYPE_BREAKPOINT		= 5,
	PERF_TYPE_MAX,			/* non-ABI */
};


/*
 * Generalized performance event event_id types, used by the
 * attr.event_id parameter of the sys_perf_event_open()
 * syscall:
 */
enum perf_hw_id {
	/*
	 * Common hardware events, generalized by the kernel:
	 */
	PERF_COUNT_HW_CPU_CYCLES		= 0,
	PERF_COUNT_HW_INSTRUCTIONS		= 1,
	PERF_COUNT_HW_CACHE_REFERENCES		= 2,
	PERF_COUNT_HW_CACHE_MISSES		= 3,
	PERF_COUNT_HW_BRANCH_INSTRUCTIONS	= 4,
	PERF_COUNT_HW_BRANCH_MISSES		= 5,
	PERF_COUNT_HW_BUS_CYCLES		= 6,

	PERF_COUNT_HW_MAX,			/* non-ABI */
};

/*
 * Generalized hardware cache events:
 *
 *       { L1-D, L1-I, LLC, ITLB, DTLB, BPU } x
 *       { read, write, prefetch } x
 *       { accesses, misses }
 */
enum perf_hw_cache_id {
	PERF_COUNT_HW_CACHE_L1D			= 0,
	PERF_COUNT_HW_CACHE_L1I			= 1,
	PERF_COUNT_HW_CACHE_LL			= 2,
	PERF_COUNT_HW_CACHE_DTLB		= 3,
	PERF_COUNT_HW_CACHE_ITLB		= 4,
	PERF_COUNT_HW_CACHE_BPU			= 5,

	PERF_COUNT_HW_CACHE_MAX,		/* non-ABI */
};

enum perf_hw_cache_op_id {
	PERF_COUNT_HW_CACHE_OP_READ		= 0,
	PERF_COUNT_HW_CACHE_OP_WRITE		= 1,
	PERF_COUNT_HW_CACHE_OP_PREFETCH		= 2,

	PERF_COUNT_HW_CACHE_OP_MAX,		/* non-ABI */
};

enum perf_hw_cache_op_result_id {
	PERF_COUNT_HW_CACHE_RESULT_ACCESS	= 0,
	PERF_COUNT_HW_CACHE_RESULT_MISS		= 1,

	PERF_COUNT_HW_CACHE_RESULT_MAX,		/* non-ABI */
};

/*
 * Special "software" events provided by the kernel, even if the hardware
 * does not support performance events. These events measure various
 * physical and sw events of the kernel (and allow the profiling of them as
 * well):
 */
enum perf_sw_ids {
	PERF_COUNT_SW_CPU_CLOCK			= 0,
	PERF_COUNT_SW_TASK_CLOCK		= 1,
	PERF_COUNT_SW_PAGE_FAULTS		= 2,
	PERF_COUNT_SW_CONTEXT_SWITCHES		= 3,
	PERF_COUNT_SW_CPU_MIGRATIONS		= 4,
	PERF_COUNT_SW_PAGE_FAULTS_MIN		= 5,
	PERF_COUNT_SW_PAGE_FAULTS_MAJ		= 6,
	PERF_COUNT_SW_ALIGNMENT_FAULTS		= 7,
	PERF_COUNT_SW_EMULATION_FAULTS		= 8,

	PERF_COUNT_SW_MAX,			/* non-ABI */
};

/*
 * Bits that can be set in attr.sample_type to request information
 * in the overflow packets.
 */
enum perf_event_sample_format {
	PERF_SAMPLE_IP			= 1U << 0,
	PERF_SAMPLE_TID			= 1U << 1,
	PERF_SAMPLE_TIME		= 1U << 2,
	PERF_SAMPLE_ADDR		= 1U << 3,
	PERF_SAMPLE_READ		= 1U << 4,
	PERF_SAMPLE_CALLCHAIN		= 1U << 5,
	PERF_SAMPLE_ID			= 1U << 6,
	PERF_SAMPLE_CPU			= 1U << 7,
	PERF_SAMPLE_PERIOD		= 1U << 8,
	PERF_SAMPLE_STREAM_ID		= 1U << 9,
	PERF_SAMPLE_RAW			= 1U << 10,
	PERF_SAMPLE_BRANCH_STACK	= 1U << 11,
	PERF_SAMPLE_REGS_USER		= 1U << 12,
	PERF_SAMPLE_STACK_USER		= 1U << 13,
	PERF_SAMPLE_WEIGHT		= 1U << 14,
	PERF_SAMPLE_DATA_SRC		= 1U << 15,

	PERF_SAMPLE_MAX			= 1U << 16,	/* non-ABI */
};

/*
 * values to load into branch_sample_type when PERF_SAMPLE_BRANCH is set
 *
 * If user does not pass priv level information, kernel uses the
 * events priv level. Branch and event priv levels do not have
 * to match.
 *
 * The branch types can be combined, however ANY covers all types
 * of branches and therefore it supersedes all the other branches.
 */
enum perf_branch_sample_type {
       PERF_SAMPLE_BRANCH_USER         = 1U << 0, /* user branches */
       PERF_SAMPLE_BRANCH_KERNEL       = 1U << 1, /* kernel branches */
       PERF_SAMPLE_BRANCH_HV           = 1U << 2, /* hypervisor branches */

       PERF_SAMPLE_BRANCH_ANY          = 1U << 3, /* any branch types */
       PERF_SAMPLE_BRANCH_ANY_CALL     = 1U << 4, /* any call branch */
       PERF_SAMPLE_BRANCH_ANY_RETURN   = 1U << 5, /* any return branch */
       PERF_SAMPLE_BRANCH_IND_CALL     = 1U << 6, /* indirect calls */

       PERF_SAMPLE_BRANCH_MAX          = 1U << 7, /* non-ABI */
};

enum perf_sample_regs_abi {
	PERF_SAMPLE_REGS_ABI_NONE	= 0,
	PERF_SAMPLE_REGS_ABI_32		= 1,
	PERF_SAMPLE_REGS_ABI_64		= 2,
};

/*
 * The format of the data returned by read() on a perf event fd,
 * as specified by attr.read_format:
 *
 * struct read_format {
 * 	{ u64		value;
 * 	  { u64		time_enabled; } && PERF_FORMAT_ENABLED
 * 	  { u64		time_running; } && PERF_FORMAT_RUNNING
 * 	  { u64		id;           } && PERF_FORMAT_ID
 * 	} && !PERF_FORMAT_GROUP
 *
 * 	{ u64		nr;
 * 	  { u64		time_enabled; } && PERF_FORMAT_ENABLED
 * 	  { u64		time_running; } && PERF_FORMAT_RUNNING
 * 	  { u64		value;
 * 	    { u64	id;           } && PERF_FORMAT_ID
 * 	  }		cntr[nr];
 * 	} && PERF_FORMAT_GROUP
 * };
 */
enum perf_event_read_format {
	PERF_FORMAT_TOTAL_TIME_ENABLED	= 1U << 0,
	PERF_FORMAT_TOTAL_TIME_RUNNING	= 1U << 1,
	PERF_FORMAT_ID			= 1U << 2,
	PERF_FORMAT_GROUP		= 1U << 3,

	PERF_FORMAT_MAX = 1U << 4, 	/* non-ABI */
};

#define PERF_ATTR_SIZE_VER0	64	/* sizeof first published struct */

/* SWIG doesn't deal well with anonymous nested structures */
#ifdef SWIG
#define SWIG_NAME(x) x
#else
#define SWIG_NAME(x)
#endif /* SWIG */

/*
 * Hardware event_id to monitor via a performance monitoring event:
 */
typedef struct perf_event_attr {
	/*
	 * Major type: hardware/software/tracepoint/etc.
	 */
	uint32_t	type;
	/*
	 * Size of the attr structure, for fwd/bwd compat.
	 */
	uint32_t	size;
	/*
	 * Type specific configuration information.
	 */
	uint64_t	config;

	union {
		uint64_t	sample_period;
		uint64_t	sample_freq;
	} sample;
//	} SWIG_NAME(sample);
	uint64_t	sample_type;
	uint64_t	read_format;

	uint64_t	disabled       :  1, /* off by default        */
			inherit	       :  1, /* children inherit it   */
			pinned	       :  1, /* must always be on PMU */
			exclusive      :  1, /* only group on PMU     */
			exclude_user   :  1, /* don't count user      */
			exclude_kernel :  1, /* ditto kernel          */
			exclude_hv     :  1, /* ditto hypervisor      */
			exclude_idle   :  1, /* don't count when idle */
			mmap           :  1, /* include mmap data     */
			comm	       :  1, /* include comm data     */
			freq           :  1, /* use freq, not period  */
			inherit_stat   :  1, /* per task counts       */
			enable_on_exec :  1, /* next exec enables     */
			task           :  1, /* trace fork/exit       */
			watermark      :  1, /* wakeup_watermark      */
			/*
			 * precise_ip:
			 *  0 - SAMPLE_IP can have arbitrary skid
			 *  1 - SAMPLE_IP must have constant skid
			 *  2 - SAMPLE_IP requested to have 0 skid
			 *  3 - SAMPLE_IP must have 0 skid
			 *
			 *  See also PERF_RECORD_MISC_EXACT_IP
			 */
			precise_ip     :  2, /* skid constraint       */
			mmap_data      :  1, /* non-exec mmap data    */
			sample_id_all  :  1, /* sample_type all events */
			exclude_host   :  1,
			exclude_guest  :  1,
			exclude_callchain_kernel : 1,
			exclude_callchain_user   : 1,
			sample_evt_id  :  1,
			__reserved_1   : 40;

	union {
		uint32_t	wakeup_events;		/* wakeup every n events */
		uint32_t	wakeup_watermark;	/* bytes before wakeup */
	} wakeup;
//	} SWIG_NAME(wakeup);

	uint32_t        bp_type;
	union {
		uint64_t        bp_addr;
		uint64_t	config1;
	} bp1;
//	} SWIG_NAME(bp1);
	union {
		uint64_t        bp_len;
		uint64_t	config2;
	} bp2;
//	} SWIG_NAME(bp2);
	uint64_t branch_sample_type;
	uint64_t sample_regs_user;
	uint32_t sample_stack_user;
	uint32_t __reserved_2;
} perf_event_attr_t;

/*
 * Ioctls that can be done on a perf event fd:
 */
#define PERF_EVENT_IOC_ENABLE		_IO ('$', 0)
#define PERF_EVENT_IOC_DISABLE		_IO ('$', 1)
#define PERF_EVENT_IOC_REFRESH		_IO ('$', 2)
#define PERF_EVENT_IOC_RESET		_IO ('$', 3)
#define PERF_EVENT_IOC_PERIOD		_IOW('$', 4, u64)
#define PERF_EVENT_IOC_SET_OUTPUT	_IO ('$', 5)
#define PERF_EVENT_IOC_SET_FILTER	_IOW('$', 6, char *)

enum perf_event_ioc_flags {
	PERF_IOC_FLAG_GROUP		= 1U << 0,
};

/*
 * Structure of the page that can be mapped via mmap
 */
struct perf_event_mmap_page {
	uint32_t	version;		/* version number of this structure */
	uint32_t	compat_version;		/* lowest version this is compat with */
	uint32_t	lock;			/* seqlock for synchronization */
	uint32_t	index;			/* hardware event identifier */
	int64_t		offset;			/* add to hardware event value */
	uint64_t	time_enabled;		/* time event active */
	uint64_t	time_running;		/* time event on cpu */
	union {
		uint64_t capabilities;
		uint64_t cap_usr_time  : 1,
			 cap_usr_rdpmc : 1,
			 cap_____res   : 62;
	};
	uint16_t pmc_width;
	uint16_t time_shift;
	uint32_t time_mult;
	uint64_t time_offset;
	uint64_t __reserved[120];

	/*
	 * Control data for the mmap() data buffer.
	 *
	 * User-space reading the @data_head value should issue an rmb(), on
	 * SMP capable platforms, after reading this value -- see
	 * perf_event_wakeup().
	 *
	 * When the mapping is PROT_WRITE the @data_tail value should be
	 * written by userspace to reflect the last read data. In this case
	 * the kernel will not over-write unread data.
	 */
	uint64_t   	data_head;		/* head in the data section */
	uint64_t	data_tail;		/* user-space written tail */
};

#define PERF_EVENT_MISC_CPUMODE_MASK	(3 << 0)
#define PERF_EVENT_MISC_CPUMODE_UNKNOWN	(0 << 0)
#define PERF_EVENT_MISC_KERNEL		(1 << 0)
#define PERF_EVENT_MISC_USER		(2 << 0)
#define PERF_EVENT_MISC_HYPERVISOR	(3 << 0)
#define PERF_RECORD_MISC_GUEST_KERNEL	(4 << 0)
#define PERF_RECORD_MISC_GUEST_USER	(5 << 0)

#define PERF_RECORD_MISC_EXACT		(1 << 14)
/*
 * Indicates that the content of PERF_SAMPLE_IP points to
 * the actual instruction that triggered the event. See also
 * perf_event_attr::precise_ip.
 */
#define PERF_RECORD_MISC_EXACT_IP               (1 << 14)
/*
 * Reserve the last bit to indicate some extended misc field
 */
#define PERF_RECORD_MISC_EXT_RESERVED		(1 << 15)

struct perf_event_header {
	uint32_t	type;
	uint16_t	misc;
	uint16_t	size;
};

enum perf_event_type {

	/*
	 * The MMAP events record the PROT_EXEC mappings so that we can
	 * correlate userspace IPs to code. They have the following structure:
	 *
	 * struct {
	 *	struct perf_event_header	header;
	 *
	 *	u32				pid, tid;
	 *	u64				addr;
	 *	u64				len;
	 *	u64				pgoff;
	 *	char				filename[];
	 * };
	 */
	PERF_RECORD_MMAP			= 1,

	/*
	 * struct {
	 * 	struct perf_event_header	header;
	 * 	u64				id;
	 * 	u64				lost;
	 * };
	 */
	PERF_RECORD_LOST			= 2,

	/*
	 * struct {
	 *	struct perf_event_header	header;
	 *
	 *	u32				pid, tid;
	 *	char				comm[];
	 * };
	 */
	PERF_RECORD_COMM			= 3,

	/*
	 * struct {
	 *	struct perf_event_header	header;
	 *	u32				pid, ppid;
	 *	u32				tid, ptid;
	 *	u64				time;
	 * };
	 */
	PERF_RECORD_EXIT			= 4,

	/*
	 * struct {
	 *	struct perf_event_header	header;
	 *	u64				time;
	 *	u64				id;
	 *	u64				stream_id;
	 * };
	 */
	PERF_RECORD_THROTTLE		= 5,
	PERF_RECORD_UNTHROTTLE		= 6,

	/*
	 * struct {
	 *	struct perf_event_header	header;
	 *	u32				pid, ppid;
	 *	u32				tid, ptid;
	 *	{ u64				time;     } && PERF_SAMPLE_TIME
	 * };
	 */
	PERF_RECORD_FORK			= 7,

	/*
	 * struct {
	 * 	struct perf_event_header	header;
	 * 	u32				pid, tid;
	 * 	struct read_format		values;
	 * };
	 */
	PERF_RECORD_READ			= 8,

	/*
	 * struct {
	 *	struct perf_event_header	header;
	 *
	 *	{ u64			ip;	  } && PERF_SAMPLE_IP
	 *	{ u32			pid, tid; } && PERF_SAMPLE_TID
	 *	{ u64			time;     } && PERF_SAMPLE_TIME
	 *	{ u64			addr;     } && PERF_SAMPLE_ADDR
	 *	{ u64			id;	  } && PERF_SAMPLE_ID
	 *	{ u64			stream_id;} && PERF_SAMPLE_STREAM_ID
	 *	{ u32			cpu, res; } && PERF_SAMPLE_CPU
	 * 	{ u64			period;   } && PERF_SAMPLE_PERIOD
	 *
	 *	{ struct read_format	values;	  } && PERF_SAMPLE_READ
	 *
	 *	{ u64			nr,
	 *	  u64			ips[nr];  } && PERF_SAMPLE_CALLCHAIN
	 *
	 * 	#
	 * 	# The RAW record below is opaque data wrt the ABI
	 * 	#
	 * 	# That is, the ABI doesn't make any promises wrt to
	 * 	# the stability of its content, it may vary depending
	 * 	# on event, hardware, kernel version and phase of
	 * 	# the moon.
	 * 	#
	 * 	# In other words, PERF_SAMPLE_RAW contents are not an ABI.
	 * 	#
	 *
	 *	{ u32			size;
	 *	  char                  data[size];}&& PERF_SAMPLE_RAW
	 *
	 *	{ u64 from, to, flags } lbr[nr];} && PERF_SAMPLE_BRANCH_STACK
	 *
	 * 	{ u64			abi; # enum perf_sample_regs_abi
	 * 	  u64			regs[weight(mask)]; } && PERF_SAMPLE_REGS_USER
	 *
	 * 	{ u64			size;
	 * 	  char			data[size];
	 * 	  u64			dyn_size; } && PERF_SAMPLE_STACK_USER
	 *	{ u64			weight;   } && PERF_SAMPLE_WEIGHT
	 *	{ u64			data_src;     } && PERF_SAMPLE_DATA_SRC
	 * };
	 */
	PERF_RECORD_SAMPLE		= 9,

	PERF_RECORD_MAX,			/* non-ABI */
};

enum perf_callchain_context {
	PERF_CONTEXT_HV			= (uint64_t)-32,
	PERF_CONTEXT_KERNEL		= (uint64_t)-128,
	PERF_CONTEXT_USER		= (uint64_t)-512,

	PERF_CONTEXT_GUEST		= (uint64_t)-2048,
	PERF_CONTEXT_GUEST_KERNEL	= (uint64_t)-2176,
	PERF_CONTEXT_GUEST_USER		= (uint64_t)-2560,

	PERF_CONTEXT_MAX		= (uint64_t)-4095,
} ;

#define PERF_FLAG_FD_NO_GROUP	(1U << 0)
#define PERF_FLAG_FD_OUTPUT	(1U << 1)
#define PERF_FLAG_PID_CGROUP		(1U << 2) /* pid=cgroup id, per-cpu mode only */

union perf_mem_data_src {
	uint64_t val;
	struct {
		uint64_t mem_op:5,	/* type of opcode */
			 mem_lvl:14,	/* memory hierarchy level */
			 mem_snoop:5,	/* snoop mode */
			 mem_lock:2,	/* lock instr */
			 mem_dtlb:7,	/* tlb access */
			 mem_rsvd:31;
	 };
};

/* type of opcode (load/store/prefetch,code) */
#define PERF_MEM_OP_NA		0x01 /* not available */
#define PERF_MEM_OP_LOAD	0x02 /* load instruction */
#define PERF_MEM_OP_STORE	0x04 /* store instruction */
#define PERF_MEM_OP_PFETCH	0x08 /* prefetch */
#define PERF_MEM_OP_EXEC	0x10 /* code (execution) */
#define PERF_MEM_OP_SHIFT	0

/* memory hierarchy (memory level, hit or miss) */
#define PERF_MEM_LVL_NA		0x01  /* not available */
#define PERF_MEM_LVL_HIT	0x02  /* hit level */
#define PERF_MEM_LVL_MISS	0x04  /* miss level  */
#define PERF_MEM_LVL_L1		0x08  /* L1 */
#define PERF_MEM_LVL_LFB	0x10  /* Line Fill Buffer */
#define PERF_MEM_LVL_L2		0x20  /* L2 hit */
#define PERF_MEM_LVL_L3		0x40  /* L3 hit */
#define PERF_MEM_LVL_LOC_RAM	0x80  /* Local DRAM */
#define PERF_MEM_LVL_REM_RAM1	0x100 /* Remote DRAM (1 hop) */
#define PERF_MEM_LVL_REM_RAM2	0x200 /* Remote DRAM (2 hops) */
#define PERF_MEM_LVL_REM_CCE1	0x400 /* Remote Cache (1 hop) */
#define PERF_MEM_LVL_REM_CCE2	0x800 /* Remote Cache (2 hops) */
#define PERF_MEM_LVL_IO		0x1000 /* I/O memory */
#define PERF_MEM_LVL_UNC	0x2000 /* Uncached memory */
#define PERF_MEM_LVL_SHIFT	5

/* snoop mode */
#define PERF_MEM_SNOOP_NA	0x01 /* not available */
#define PERF_MEM_SNOOP_NONE	0x02 /* no snoop */
#define PERF_MEM_SNOOP_HIT	0x04 /* snoop hit */
#define PERF_MEM_SNOOP_MISS	0x08 /* snoop miss */
#define PERF_MEM_SNOOP_HITM	0x10 /* snoop hit modified */
#define PERF_MEM_SNOOP_SHIFT	19

/* locked instruction */
#define PERF_MEM_LOCK_NA	0x01 /* not available */
#define PERF_MEM_LOCK_LOCKED	0x02 /* locked transaction */
#define PERF_MEM_LOCK_SHIFT	24

/* TLB access */
#define PERF_MEM_TLB_NA		0x01 /* not available */
#define PERF_MEM_TLB_HIT	0x02 /* hit level */
#define PERF_MEM_TLB_MISS	0x04 /* miss level */
#define PERF_MEM_TLB_L1		0x08 /* L1 */
#define PERF_MEM_TLB_L2		0x10 /* L2 */
#define PERF_MEM_TLB_WK		0x20 /* Hardware Walker*/
#define PERF_MEM_TLB_OS		0x40 /* OS fault handler */
#define PERF_MEM_TLB_SHIFT	26

#define PERF_MEM_S(a, s) \
	(((u64)PERF_MEM_##a##_##s) << PERF_MEM_##a##_SHIFT)

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#ifndef __NR_perf_event_open
#ifdef __x86_64__
# define __NR_perf_event_open	298
#endif

#ifdef __i386__
# define __NR_perf_event_open 336
#endif

#ifdef __powerpc__
#define __NR_perf_event_open 319
#endif

#ifdef __arm__
#if defined(__ARM_EABI__) || defined(__thumb__)
#define __NR_perf_event_open 364
#else
#define __NR_perf_event_open (0x900000+364)
#endif
#endif
#endif /* __NR_perf_event_open */

static inline int
perf_event_open(
	struct perf_event_attr		*hw_event_uptr,
	pid_t				pid,
	int				cpu,
	int				group_fd,
	unsigned long			flags)
{
	return syscall(
		__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd, flags);
}
#endif
#endif /* _LINUX_PERF_EVENT_H */
