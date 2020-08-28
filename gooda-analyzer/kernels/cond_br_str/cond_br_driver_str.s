	.file	"cond_br_driver_str.c"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"this program supports only up to %d CPUs"
	.text
	.p2align 4,,15
	.type	pin_cpu.constprop.1, @function
pin_cpu.constprop.1:
.LFB77:
	.cfi_startproc
	subq	$2072, %rsp
	.cfi_def_cfa_offset 2080
	movl	%edi, %esi
	movl	$256, %ecx
	movq	%fs:40, %rax
	movq	%rax, 2056(%rsp)
	xorl	%eax, %eax
	movq	%rsp, %rdx
	cmpl	$2047, %esi
	movq	%rdx, %rdi
	rep stosq
	ja	.L6
	movl	%esi, %ecx
	movl	%esi, %eax
	movl	$1, %edi
	salq	%cl, %rdi
	shrl	$6, %eax
	movq	%rdx, %rcx
	movq	%rdi, (%rsp,%rax,8)
	movl	$2048, %edx
	xorl	%esi, %esi
	xorl	%eax, %eax
	movl	$203, %edi
	call	syscall@PLT
	movq	2056(%rsp), %rdx
	xorq	%fs:40, %rdx
	jne	.L7
	addq	$2072, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L6:
	.cfi_restore_state
	leaq	.LC0(%rip), %rsi
	movl	$2048, %edx
	movl	$1, %edi
	call	errx@PLT
.L7:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE77:
	.size	pin_cpu.constprop.1, .-pin_cpu.constprop.1
	.p2align 4,,15
	.globl	rndm_list
	.type	rndm_list, @function
rndm_list:
.LFB73:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movslq	%esi, %rax
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rax, %r14
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	salq	$2, %rax
	movq	%rax, %rbx
	movl	$100, %ebp
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	movq	%rdi, 16(%rsp)
	movq	%rax, %rdi
	movq	%rax, 24(%rsp)
	call	malloc@PLT
	movq	%rbx, %rdi
	movq	%rax, %r13
	call	malloc@PLT
	movq	%rax, %r12
	.p2align 4,,10
	.p2align 3
.L9:
	call	drand48@PLT
	subl	$1, %ebp
	jne	.L9
	leal	-1(%r14), %ebx
	testl	%ebx, %ebx
	jle	.L10
	movq	%r13, %rax
	subl	$2, %r14d
	shrq	$2, %rax
	negq	%rax
	andl	$3, %eax
	cmpl	$5, %r14d
	jbe	.L21
	xorl	%ecx, %ecx
	testl	%eax, %eax
	je	.L12
	cmpl	$1, %eax
	movl	$1, 0(%r13)
	movl	$1, %ecx
	je	.L12
	cmpl	$3, %eax
	movl	$2, 4(%r13)
	movl	$2, %ecx
	jne	.L12
	movl	$3, 8(%r13)
	movl	$3, %ecx
.L12:
	movl	%ecx, 8(%rsp)
	movl	%ebx, %esi
	movd	8(%rsp), %xmm6
	subl	%eax, %esi
	leaq	0(%r13,%rax,4), %rax
	movl	%esi, %edx
	movdqa	.LC2(%rip), %xmm3
	pshufd	$0, %xmm6, %xmm0
	shrl	$2, %edx
	movdqa	.LC3(%rip), %xmm2
	paddd	.LC1(%rip), %xmm0
	.p2align 4,,10
	.p2align 3
.L14:
	movdqa	%xmm0, %xmm1
	addl	$1, %ebp
	paddd	%xmm3, %xmm0
	addq	$16, %rax
	paddd	%xmm2, %xmm1
	movaps	%xmm1, -16(%rax)
	cmpl	%ebp, %edx
	ja	.L14
	movl	%esi, %edx
	andl	$-4, %edx
	cmpl	%edx, %esi
	leal	(%rdx,%rcx), %eax
	je	.L15
.L11:
	leal	1(%rax), %edx
	movslq	%eax, %rcx
	cmpl	%edx, %ebx
	movl	%edx, 0(%r13,%rcx,4)
	jle	.L15
	leal	2(%rax), %ecx
	movslq	%edx, %rdx
	cmpl	%ecx, %ebx
	movl	%ecx, 0(%r13,%rdx,4)
	jle	.L15
	leal	3(%rax), %edx
	movslq	%ecx, %rcx
	cmpl	%edx, %ebx
	movl	%edx, 0(%r13,%rcx,4)
	jle	.L15
	leal	4(%rax), %ecx
	movslq	%edx, %rdx
	cmpl	%ecx, %ebx
	movl	%ecx, 0(%r13,%rdx,4)
	jle	.L15
	leal	5(%rax), %edx
	movslq	%ecx, %rcx
	cmpl	%edx, %ebx
	movl	%edx, 0(%r13,%rcx,4)
	jle	.L15
	movslq	%edx, %rdx
	addl	$6, %eax
	movl	%eax, 0(%r13,%rdx,4)
.L15:
	leaq	4(,%r14,4), %rdx
	movl	$255, %esi
	movq	%r12, %rdi
	leal	-1(%rbx), %ebp
	leaq	4(%r13), %r15
	call	memset@PLT
	movq	16(%rsp), %r14
	.p2align 4,,10
	.p2align 3
.L20:
	call	drand48@PLT
	pxor	%xmm5, %xmm5
	cvtsi2sd	%ebx, %xmm5
	addl	$1, %ebx
	mulsd	%xmm5, %xmm0
	movsd	%xmm5, 8(%rsp)
	cvttsd2si	%xmm0, %eax
	cmpl	%ebx, %eax
	jne	.L16
	.p2align 4,,10
	.p2align 3
.L17:
	call	drand48@PLT
	mulsd	8(%rsp), %xmm0
	cvttsd2si	%xmm0, %eax
	cmpl	%ebx, %eax
	je	.L17
.L16:
	cltq
	xorl	%ecx, %ecx
	leaq	0(%r13,%rax,4), %rax
	movl	(%rax), %edx
	movl	$-1, (%rax)
	movl	%ebp, %eax
	leaq	(%r15,%rax,4), %rdi
	movq	%r13, %rax
	movl	%edx, (%r14)
	.p2align 4,,10
	.p2align 3
.L19:
	movl	(%rax), %edx
	cmpl	$-1, %edx
	je	.L18
	movslq	%ecx, %rsi
	addl	$1, %ecx
	movl	%edx, (%r12,%rsi,4)
.L18:
	addq	$4, %rax
	cmpq	%rdi, %rax
	jne	.L19
	testl	%ebp, %ebp
	movl	%ebp, %ebx
	je	.L10
	leal	-1(%rbp), %eax
	movq	%r12, %rsi
	movq	%r13, %rdi
	addq	$4, %r14
	leaq	4(,%rax,4), %rdx
	movq	%rax, %rbp
	call	memcpy@PLT
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L10:
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdi
	movl	$0, -4(%rax,%rdi)
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L21:
	.cfi_restore_state
	xorl	%eax, %eax
	jmp	.L11
	.cfi_endproc
.LFE73:
	.size	rndm_list, .-rndm_list
	.section	.rodata.str1.8
	.align 8
.LC4:
	.string	" rndm_walker requires 6 arguments and has a seventh optional argument\n"
	.align 8
.LC5:
	.string	" -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n"
	.align 8
.LC6:
	.string	" -rM  -r signifies run M indicates which core the pointer walker should be executed on\n"
	.align 8
.LC7:
	.string	" -lN  -l signifies inner loop depth N indicates the number of calls to the top of the conditional branch test kernel,\n"
	.align 8
.LC8:
	.string	" this size controls the buffer size and thus in which level of cache/memory the buffers reside. \n"
	.align 8
.LC9:
	.string	" The HW prefetchers will pull the buffers to L1d unless they are disabled. When disabled one can test speculative load counts by level\n"
	.align 8
.LC10:
	.string	" [-m] value increases the number of calls to triad by a multiplier = value\n"
	.align 8
.LC11:
	.string	" [-T] allowed values 0,1,2, default is 0 meaning random test arg, 1 means test_arg = 1 2 means test arg = 2 \n"
	.text
	.p2align 4,,15
	.globl	usage
	.type	usage, @function
usage:
.LFB74:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movq	stderr(%rip), %rcx
	leaq	.LC4(%rip), %rdi
	movl	$70, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC5(%rip), %rdi
	movl	$96, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC6(%rip), %rdi
	movl	$87, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC7(%rip), %rdi
	movl	$118, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC8(%rip), %rdi
	movl	$97, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC9(%rip), %rdi
	movl	$135, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC10(%rip), %rdi
	movl	$75, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movq	stderr(%rip), %rcx
	leaq	.LC11(%rip), %rdi
	movl	$109, %edx
	movl	$1, %esi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	fwrite@PLT
	.cfi_endproc
.LFE74:
	.size	usage, .-usage
	.section	.rodata.str1.8
	.align 8
.LC12:
	.string	"the random conditional branch code requires at least 3 arguments  there were %d\n"
	.align 8
.LC13:
	.string	"insufficient invocation arguments"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC14:
	.string	"unknown option %c"
.LC15:
	.string	"i:r:l:m:T:C"
	.section	.rodata.str1.8
	.align 8
.LC16:
	.string	"invalid value for -T was %d must be 0 or 1 or 2\n"
	.section	.rodata.str1.1
.LC17:
	.string	"size_t in %zd bytes\n"
.LC18:
	.string	"failed to set affinity"
.LC19:
	.string	" process pinned to core %d\n"
	.section	.rodata.str1.8
	.align 8
.LC20:
	.string	" failed to malloc index array for line_count of %d\n"
	.section	.rodata.str1.1
.LC21:
	.string	"failed to malloc index"
	.section	.rodata.str1.8
	.align 8
.LC23:
	.string	" failed to malloc buf1 for len*sizeof(double) of %zd\n"
	.section	.rodata.str1.1
.LC24:
	.string	"failed to malloc buf1"
	.section	.rodata.str1.8
	.align 8
.LC25:
	.string	" failed to malloc buf2 for line_count*len*sizeof(double) of %zd\n"
	.section	.rodata.str1.1
.LC26:
	.string	"failed to malloc buf2"
.LC31:
	.string	"cannot set cpu run affinity"
	.section	.rodata.str1.8
	.align 8
.LC32:
	.string	" process pinned to core %d to run\n"
	.align 8
.LC33:
	.string	" calling conditional br loop %d times which loops  %d times on set of %d functions\n"
	.section	.rodata.str1.1
.LC34:
	.string	" done"
.LC35:
	.string	" run time = %zd\n"
.LC36:
	.string	" sum_run time = %zd\n"
.LC37:
	.string	" gettimeofday_run time = %zd\n"
	.section	.rodata.str1.8
	.align 8
.LC38:
	.string	" init_run = %zd, init_end = %zd, call_start = %zd\n"
	.section	.rodata.str1.1
.LC39:
	.string	"total = %lu\n"
	.section	.rodata.str1.8
	.align 8
.LC41:
	.string	" average cycles per iteration = %f\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB75:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	%edi, %ebp
	subq	$4000264, %rsp
	.cfi_def_cfa_offset 4000320
	movdqa	.LC42(%rip), %xmm0
	movq	%fs:40, %rax
	movq	%rax, 4000248(%rsp)
	xorl	%eax, %eax
	movaps	%xmm0, 4000192(%rsp)
	movb	$0, 4000208(%rsp)
	movb	$0, 4000240(%rsp)
	movdqa	.LC43(%rip), %xmm0
	movaps	%xmm0, 4000224(%rsp)
#APP
# 54 "cond_br_driver_str.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	salq	$32, %rdx
	movl	%eax, %eax
	orq	%rax, %rdx
	cmpl	$2, %edi
	movq	%rdx, 56(%rsp)
	jle	.L105
	leaq	.L45(%rip), %rbx
	movq	%rsi, %r13
	xorl	%r12d, %r12d
	movl	$1, 28(%rsp)
	.p2align 4,,10
	.p2align 3
.L42:
	leaq	.LC15(%rip), %rdx
	movq	%r13, %rsi
	movl	%ebp, %edi
	call	getopt@PLT
	cmpl	$-1, %eax
	je	.L106
	leal	-84(%rax), %edx
	cmpl	$30, %edx
	ja	.L43
	movslq	(%rbx,%rdx,4), %rdx
	addq	%rbx, %rdx
	jmp	*%rdx
	.section	.rodata
	.align 4
	.align 4
.L45:
	.long	.L44-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L46-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L47-.L45
	.long	.L48-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L43-.L45
	.long	.L49-.L45
	.section	.text.startup
	.p2align 4,,10
	.p2align 3
.L49:
	movq	optarg(%rip), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movl	%eax, %r14d
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L48:
	movq	optarg(%rip), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movl	%eax, 28(%rsp)
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L47:
	movq	optarg(%rip), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movl	%eax, 24(%rsp)
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L46:
	movq	optarg(%rip), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movl	%eax, %r15d
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L44:
	movq	optarg(%rip), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movl	%eax, %r12d
	jmp	.L42
	.p2align 4,,10
	.p2align 3
.L106:
	cmpl	$2, %r12d
	movq	stderr(%rip), %rdi
	ja	.L107
	leaq	.LC17(%rip), %rdx
	movl	$8, %ecx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movl	%r15d, %edi
	call	pin_cpu.constprop.1
	cmpl	$-1, %eax
	je	.L108
	movq	stderr(%rip), %rdi
	leaq	.LC19(%rip), %rdx
	movl	%r15d, %ecx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movslq	24(%rsp), %rdi
	movq	%rdi, %rbx
	salq	$2, %rdi
	call	malloc@PLT
	testq	%rax, %rax
	je	.L54
	testl	%ebx, %ebx
	jle	.L56
	cmpl	$1, %r12d
	leaq	112(%rsp), %r13
	movl	24(%rsp), %eax
	je	.L57
	leal	-1(%rax), %edx
	leaq	8(%r13), %r15
	leaq	4000192(%rsp), %rax
	leaq	4000224(%rsp), %rbp
	leaq	(%r15,%rdx,8), %rbx
	movq	%rax, 8(%rsp)
	jmp	.L62
	.p2align 4,,10
	.p2align 3
.L109:
	testl	%r12d, %r12d
	jne	.L61
	ucomisd	.LC22(%rip), %xmm0
	ja	.L58
	movq	8(%rsp), %rax
	movq	%rax, 0(%r13)
.L61:
	cmpq	%rbx, %r15
	movq	%r15, %r13
	je	.L56
.L110:
	addq	$8, %r15
.L62:
	call	drand48@PLT
	cmpl	$2, %r12d
	jne	.L109
.L58:
	cmpq	%rbx, %r15
	movq	%rbp, 0(%r13)
	movq	%r15, %r13
	jne	.L110
.L56:
	xorl	%r9d, %r9d
	xorl	%edi, %edi
	movl	$-1, %r8d
	movl	$34, %ecx
	movl	$3, %edx
	movl	$800, %esi
	call	mmap@PLT
	testq	%rax, %rax
	movq	%rax, %r12
	je	.L111
	imull	$100, 24(%rsp), %ebx
	xorl	%r9d, %r9d
	xorl	%edi, %edi
	movl	$-1, %r8d
	movl	$34, %ecx
	movl	$3, %edx
	movslq	%ebx, %rbp
	salq	$3, %rbp
	movq	%rbp, %rsi
	call	mmap@PLT
	testq	%rax, %rax
	movq	%rax, 40(%rsp)
	je	.L112
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	movl	$34, %ecx
	movl	$-1, %r8d
	movl	$3, %edx
	movq	%rbp, %rsi
	call	mmap@PLT
	leaq	8(%r12), %rdi
	movq	%r12, %rcx
	movq	%rax, 32(%rsp)
	xorl	%eax, %eax
	movq	$0, (%r12)
	movq	$0, 792(%r12)
	andq	$-8, %rdi
	subq	%rdi, %rcx
	addl	$800, %ecx
	shrl	$3, %ecx
	testl	%ebx, %ebx
	rep stosq
	jle	.L67
	movq	40(%rsp), %rcx
	leal	-1(%rbx), %esi
	movq	%rcx, %rax
	shrq	$3, %rax
	andl	$1, %eax
	cmpl	$4, %esi
	jbe	.L88
	xorl	%r8d, %r8d
	testl	%eax, %eax
	je	.L69
	movq	.LC27(%rip), %rdi
	movl	$1, %r8d
	movq	%rdi, (%rcx)
.L69:
	movl	%ebx, %edi
	movq	40(%rsp), %rdx
	subl	%eax, %edi
	movapd	.LC28(%rip), %xmm0
	movl	%edi, %ecx
	leaq	(%rdx,%rax,8), %rdx
	shrl	%ecx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L70:
	addl	$1, %eax
	addq	$16, %rdx
	movaps	%xmm0, -16(%rdx)
	cmpl	%eax, %ecx
	ja	.L70
	movl	%edi, %edx
	andl	$-2, %edx
	cmpl	%edx, %edi
	leal	(%rdx,%r8), %eax
	je	.L113
.L68:
	movq	40(%rsp), %rcx
	movsd	.LC27(%rip), %xmm0
	movslq	%eax, %rdx
	movsd	%xmm0, (%rcx,%rdx,8)
	leal	1(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L72
	movslq	%edx, %rdx
	movsd	%xmm0, (%rcx,%rdx,8)
	leal	2(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L72
	movslq	%edx, %rdx
	movsd	%xmm0, (%rcx,%rdx,8)
	leal	3(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L72
	addl	$4, %eax
	movslq	%edx, %rdx
	cmpl	%eax, %ebx
	movsd	%xmm0, (%rcx,%rdx,8)
	jle	.L72
	cltq
	movsd	%xmm0, (%rcx,%rax,8)
.L72:
	movq	32(%rsp), %rdx
	shrq	$3, %rdx
	andl	$1, %edx
	cmpl	$4, %esi
	jbe	.L90
.L85:
	xorl	%edi, %edi
	testl	%edx, %edx
	je	.L74
	movq	32(%rsp), %rax
	movq	.LC29(%rip), %rsi
	movl	$1, %edi
	movq	%rsi, (%rax)
.L74:
	movq	32(%rsp), %rax
	movl	%ebx, %esi
	subl	%edx, %esi
	movapd	.LC30(%rip), %xmm0
	movl	%esi, %ecx
	leaq	(%rax,%rdx,8), %rdx
	shrl	%ecx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L75:
	addl	$1, %eax
	addq	$16, %rdx
	movaps	%xmm0, -16(%rdx)
	cmpl	%eax, %ecx
	ja	.L75
	movl	%esi, %edx
	andl	$-2, %edx
	cmpl	%esi, %edx
	leal	(%rdx,%rdi), %eax
	je	.L67
.L73:
	movq	32(%rsp), %rsi
	movsd	.LC29(%rip), %xmm0
	movslq	%eax, %rdx
	movsd	%xmm0, (%rsi,%rdx,8)
	leal	1(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L67
	movslq	%edx, %rdx
	movsd	%xmm0, (%rsi,%rdx,8)
	leal	2(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L67
	movslq	%edx, %rdx
	movsd	%xmm0, (%rsi,%rdx,8)
	leal	3(%rax), %edx
	cmpl	%edx, %ebx
	jle	.L67
	addl	$4, %eax
	movslq	%edx, %rdx
	cmpl	%eax, %ebx
	movsd	%xmm0, (%rsi,%rdx,8)
	jle	.L67
	cltq
	movsd	%xmm0, (%rsi,%rax,8)
.L67:
#APP
# 54 "cond_br_driver_str.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	salq	$32, %rdx
	movl	%eax, %eax
	movl	%r14d, %edi
	orq	%rax, %rdx
	movq	%rdx, 64(%rsp)
	call	pin_cpu.constprop.1
	cmpl	$-1, %eax
	je	.L114
	leaq	.LC32(%rip), %rsi
	movl	%r14d, %edx
	movl	$1, %edi
	xorl	%eax, %eax
	call	__printf_chk@PLT
	movl	24(%rsp), %r14d
	movl	28(%rsp), %ebx
	leaq	.LC33(%rip), %rsi
	movl	$100, %ecx
	movl	$1, %edi
	xorl	%eax, %eax
	movl	%r14d, %r8d
	movl	%ebx, %edx
	call	__printf_chk@PLT
#APP
# 54 "cond_br_driver_str.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	salq	$32, %rdx
	movl	%eax, %eax
	leaq	80(%rsp), %rdi
	orq	%rax, %rdx
	xorl	%esi, %esi
	movq	%rdx, 72(%rsp)
	call	gettimeofday@PLT
	testl	%ebx, %ebx
	jle	.L92
	leaq	112(%rsp), %rcx
	leal	-1(%r14), %eax
	xorl	%ebp, %ebp
	movl	$0, 20(%rsp)
	leaq	8(%rcx,%rax,8), %r14
	leaq	4000192(%rsp), %rax
	movq	%rcx, 48(%rsp)
	movq	%rax, 8(%rsp)
	.p2align 4,,10
	.p2align 3
.L79:
	movl	24(%rsp), %eax
	testl	%eax, %eax
	jle	.L82
	movq	40(%rsp), %r13
	movq	32(%rsp), %r15
	movq	48(%rsp), %rbx
	.p2align 4,,10
	.p2align 3
.L80:
	movq	(%rbx), %rsi
	movq	8(%rsp), %rdx
	movq	%r15, %r9
	movq	%r13, %r8
	movq	%r12, %rcx
	movl	$100, %edi
	addq	$8, %rbx
	addq	$800, %r13
	addq	$800, %r15
	call	triad@PLT
	addq	%rax, %rbp
	cmpq	%r14, %rbx
	jne	.L80
.L82:
	addl	$1, 20(%rsp)
	movl	20(%rsp), %eax
	cmpl	%eax, 28(%rsp)
	jne	.L79
.L78:
	leaq	.LC34(%rip), %rdi
	call	puts@PLT
#APP
# 54 "cond_br_driver_str.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	salq	$32, %rdx
	leaq	96(%rsp), %rdi
	xorl	%esi, %esi
	movq	%rdx, %rbx
	movl	%eax, %eax
	orq	%rax, %rbx
	call	gettimeofday@PLT
	movq	96(%rsp), %r12
	subq	80(%rsp), %r12
	leaq	.LC35(%rip), %rsi
	movq	72(%rsp), %r14
	movl	$1, %edi
	xorl	%eax, %eax
	imulq	$1000000, %r12, %r12
	subq	%r14, %rbx
	addq	104(%rsp), %r12
	movq	%rbx, %rdx
	subq	88(%rsp), %r12
	call	__printf_chk@PLT
	leaq	.LC36(%rip), %rsi
	xorl	%edx, %edx
	movl	$1, %edi
	xorl	%eax, %eax
	call	__printf_chk@PLT
	leaq	.LC37(%rip), %rsi
	movq	%r12, %rdx
	movl	$1, %edi
	xorl	%eax, %eax
	call	__printf_chk@PLT
	movq	64(%rsp), %rcx
	leaq	.LC38(%rip), %rsi
	movq	%r14, %r8
	movl	$1, %edi
	xorl	%eax, %eax
	movq	%rcx, %rdx
	subq	56(%rsp), %rdx
	call	__printf_chk@PLT
	leaq	.LC39(%rip), %rsi
	xorl	%eax, %eax
	movq	%rbp, %rdx
	movl	$1, %edi
	call	__printf_chk@PLT
	testq	%rbx, %rbx
	js	.L83
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rbx, %xmm0
.L84:
	pxor	%xmm1, %xmm1
	leaq	.LC41(%rip), %rsi
	movl	$1, %edi
	movl	$1, %eax
	cvtsi2sd	28(%rsp), %xmm1
	mulsd	.LC40(%rip), %xmm1
	divsd	%xmm1, %xmm0
	call	__printf_chk@PLT
	xorl	%eax, %eax
	movq	4000248(%rsp), %rsi
	xorq	%fs:40, %rsi
	jne	.L115
	addq	$4000264, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L83:
	.cfi_restore_state
	movq	%rbx, %rax
	pxor	%xmm0, %xmm0
	shrq	%rax
	andl	$1, %ebx
	orq	%rbx, %rax
	cvtsi2sdq	%rax, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L84
.L57:
	leaq	120(%rsp), %r12
	subl	$1, %eax
	leaq	4000192(%rsp), %rbx
	leaq	(%r12,%rax,8), %rbp
	jmp	.L64
	.p2align 4,,10
	.p2align 3
.L116:
	addq	$8, %r12
.L64:
	call	drand48@PLT
	cmpq	%rbp, %r12
	movq	%rbx, 0(%r13)
	movq	%r12, %r13
	jne	.L116
	jmp	.L56
.L113:
	movq	32(%rsp), %rdx
	shrq	$3, %rdx
	andl	$1, %edx
	jmp	.L85
.L92:
	xorl	%ebp, %ebp
	jmp	.L78
.L88:
	xorl	%eax, %eax
	jmp	.L68
.L90:
	xorl	%eax, %eax
	jmp	.L73
.L105:
	movl	%edi, %ecx
	movq	stderr(%rip), %rdi
	leaq	.LC12(%rip), %rdx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	xorl	%eax, %eax
	call	usage
	leaq	.LC13(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L54:
	movq	stderr(%rip), %rdi
	movl	24(%rsp), %ecx
	leaq	.LC20(%rip), %rdx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	leaq	.LC21(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L112:
	movq	stderr(%rip), %rdi
	leaq	.LC25(%rip), %rdx
	movl	$1, %esi
	movq	%rbp, %rcx
	call	__fprintf_chk@PLT
	leaq	.LC26(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L111:
	movq	stderr(%rip), %rdi
	leaq	.LC23(%rip), %rdx
	movl	$1, %esi
	movl	$800, %ecx
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	leaq	.LC24(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L108:
	leaq	.LC18(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L115:
	call	__stack_chk_fail@PLT
.L114:
	leaq	.LC31(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
.L107:
	leaq	.LC16(%rip), %rdx
	movl	%r12d, %ecx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movl	$1, %edi
	call	exit@PLT
.L43:
	leaq	.LC14(%rip), %rsi
	movl	%eax, %edx
	movl	$1, %edi
	xorl	%eax, %eax
	call	err@PLT
	.cfi_endproc
.LFE75:
	.size	main, .-main
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC1:
	.long	0
	.long	1
	.long	2
	.long	3
	.align 16
.LC2:
	.long	4
	.long	4
	.long	4
	.long	4
	.align 16
.LC3:
	.long	1
	.long	1
	.long	1
	.long	1
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC22:
	.long	0
	.long	1071644672
	.align 8
.LC27:
	.long	0
	.long	1072693248
	.section	.rodata.cst16
	.align 16
.LC28:
	.long	0
	.long	1072693248
	.long	0
	.long	1072693248
	.section	.rodata.cst8
	.align 8
.LC29:
	.long	0
	.long	1073741824
	.section	.rodata.cst16
	.align 16
.LC30:
	.long	0
	.long	1073741824
	.long	0
	.long	1073741824
	.section	.rodata.cst8
	.align 8
.LC40:
	.long	0
	.long	1092519076
	.section	.rodata.cst16
	.align 16
.LC42:
	.quad	6877956914603976820
	.quad	3557675954559153267
	.align 16
.LC43:
	.quad	6877956914603976820
	.quad	3629733548597081203
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
