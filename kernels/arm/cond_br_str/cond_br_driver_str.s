	.arch armv8-a
	.file	"cond_br_driver_str.c"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"this program supports only up to %d CPUs"
	.text
	.align	2
	.p2align 3,,7
	.type	pin_cpu.constprop.0, %function
pin_cpu.constprop.0:
.LFB76:
	.cfi_startproc
	sub	sp, sp, #2096
	.cfi_def_cfa_offset 2096
	mov	x2, 2048
	add	x3, sp, 40
	mov	w1, 0
	stp	x29, x30, [sp]
	.cfi_offset 29, -2096
	.cfi_offset 30, -2088
	mov	x29, sp
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -2080
	.cfi_offset 20, -2072
	adrp	x20, :got:__stack_chk_guard
	mov	w19, w0
	ldr	x20, [x20, #:got_lo12:__stack_chk_guard]
	mov	x0, x3
	ldr	x3, [x20]
	str	x3, [sp, 2088]
	mov	x3,0
	bl	memset
	cmp	w19, 2047
	bhi	.L6
	mov	x3, x0
	lsr	w4, w19, 6
	mov	x0, 1
	mov	x2, 2048
	lsl	x19, x0, x19
	mov	w1, 0
	mov	x0, 122
	str	x19, [x3, x4, lsl 3]
	bl	syscall
	ldr	x2, [sp, 2088]
	ldr	x1, [x20]
	eor	x1, x2, x1
	cbnz	x1, .L7
	ldp	x29, x30, [sp]
	ldp	x19, x20, [sp, 16]
	add	sp, sp, 2096
	.cfi_remember_state
	.cfi_restore 29
	.cfi_restore 30
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
.L6:
	.cfi_restore_state
	adrp	x1, .LC0
	mov	w2, 2048
	add	x1, x1, :lo12:.LC0
	mov	w0, 1
	bl	errx
.L7:
	bl	__stack_chk_fail
	.cfi_endproc
.LFE76:
	.size	pin_cpu.constprop.0, .-pin_cpu.constprop.0
	.align	2
	.p2align 3,,7
	.global	rndm_list
	.type	rndm_list, %function
rndm_list:
.LFB72:
	.cfi_startproc
	stp	x29, x30, [sp, -96]!
	.cfi_def_cfa_offset 96
	.cfi_offset 29, -96
	.cfi_offset 30, -88
	mov	x29, sp
	stp	x25, x26, [sp, 64]
	.cfi_offset 25, -32
	.cfi_offset 26, -24
	sbfiz	x25, x1, 2, 32
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -80
	.cfi_offset 20, -72
	mov	w19, 100
	stp	x21, x22, [sp, 32]
	.cfi_offset 21, -64
	.cfi_offset 22, -56
	mov	w22, w1
	stp	x23, x24, [sp, 48]
	.cfi_offset 23, -48
	.cfi_offset 24, -40
	mov	x24, x0
	mov	x0, x25
	bl	malloc
	mov	x20, x0
	mov	x0, x25
	bl	malloc
	mov	x21, x0
	.p2align 3,,7
.L9:
	bl	drand48
	subs	w19, w19, #1
	bne	.L9
	sub	w19, w22, #1
	cmp	w19, 0
	ble	.L10
	str	x27, [sp, 80]
	.cfi_offset 27, -16
	sub	w2, w22, #2
	str	d8, [sp, 88]
	.cfi_offset 72, -8
	cmp	w2, 3
	bls	.L22
	adrp	x3, .LC1
	lsr	w1, w19, 2
	movi	v3.4s, 0x4
	mov	x0, x20
	ldr	q1, [x3, #:lo12:.LC1]
	add	x1, x20, x1, uxtw 4
	movi	v2.4s, 0x1
	.p2align 3,,7
.L12:
	mov	v0.16b, v1.16b
	add	v1.4s, v1.4s, v3.4s
	add	v0.4s, v0.4s, v2.4s
	str	q0, [x0], 16
	cmp	x0, x1
	bne	.L12
	tst	x19, 3
	and	w0, w19, -4
	beq	.L13
.L11:
	add	w1, w0, 1
	str	w1, [x20, w0, sxtw 2]
	cmp	w19, w1
	sbfiz	x1, x0, 2, 32
	ble	.L13
	add	x1, x20, x1
	add	w3, w0, 2
	cmp	w19, w3
	str	w3, [x1, 4]
	ble	.L13
	add	w3, w0, 3
	str	w3, [x1, 8]
	cmp	w3, w19
	bge	.L13
	add	w0, w0, 4
	str	w0, [x1, 12]
.L13:
	ubfiz	x2, x2, 2, 32
	mov	w1, 255
	add	x2, x2, 4
	mov	x0, x21
	bl	memset
	mov	x23, x24
	mov	w26, -1
	bl	drand48
	scvtf	d8, w19
	fmul	d0, d0, d8
	fcvtzs	w27, d0
	cmp	w27, w22
	bne	.L23
	.p2align 3,,7
.L15:
	bl	drand48
	fmul	d0, d8, d0
	fcvtzs	w1, d0
	cmp	w27, w1
	beq	.L15
.L14:
	sxtw	x1, w1
	cmp	w19, 0
	ldr	w0, [x20, x1, lsl 2]
	str	w0, [x23]
	str	w26, [x20, x1, lsl 2]
	ble	.L16
	mov	x0, 0
	mov	w2, 0
	.p2align 3,,7
.L18:
	ldr	w1, [x20, x0, lsl 2]
	add	x0, x0, 1
	cmn	w1, #1
	beq	.L17
	str	w1, [x21, w2, sxtw 2]
	add	w2, w2, 1
.L17:
	cmp	w19, w0
	bgt	.L18
.L16:
	sub	w27, w19, #1
	cmp	w27, 0
	ble	.L19
	sub	w2, w19, #2
	sub	w22, w22, #1
	add	x2, x2, 1
	add	x23, x23, 4
	mov	x1, x21
	mov	x0, x20
	lsl	x2, x2, 2
	bl	memcpy
.L20:
	mov	w19, w27
	bl	drand48
	scvtf	d8, w19
	fmul	d0, d0, d8
	fcvtzs	w27, d0
	cmp	w27, w22
	beq	.L15
.L23:
	mov	w1, w27
	b	.L14
.L19:
	sub	w22, w22, #1
	add	x23, x23, 4
	bne	.L20
	ldr	x27, [sp, 80]
	.cfi_restore 27
	ldr	d8, [sp, 88]
	.cfi_restore 72
.L10:
	add	x24, x24, x25
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x25, x26, [sp, 64]
	str	wzr, [x24, -4]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 96
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
.L22:
	.cfi_def_cfa_offset 96
	.cfi_offset 19, -80
	.cfi_offset 20, -72
	.cfi_offset 21, -64
	.cfi_offset 22, -56
	.cfi_offset 23, -48
	.cfi_offset 24, -40
	.cfi_offset 25, -32
	.cfi_offset 26, -24
	.cfi_offset 27, -16
	.cfi_offset 29, -96
	.cfi_offset 30, -88
	.cfi_offset 72, -8
	mov	w0, 0
	b	.L11
	.cfi_endproc
.LFE72:
	.size	rndm_list, .-rndm_list
	.section	.rodata.str1.8
	.align	3
.LC2:
	.string	" rndm_walker requires 6 arguments and has a seventh optional argument\n"
	.align	3
.LC3:
	.string	" -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n"
	.align	3
.LC4:
	.string	" -rM  -r signifies run M indicates which core the pointer walker should be executed on\n"
	.align	3
.LC5:
	.string	" -lN  -l signifies inner loop depth N indicates the number of calls to the top of the conditional branch test kernel,\n"
	.align	3
.LC6:
	.string	" this size controls the buffer size and thus in which level of cache/memory the buffers reside. \n"
	.align	3
.LC7:
	.string	" The HW prefetchers will pull the buffers to L1d unless they are disabled. When disabled one can test speculative load counts by level\n"
	.align	3
.LC8:
	.string	" [-m] value increases the number of calls to triad by a multiplier = value\n"
	.align	3
.LC9:
	.string	" [-T] allowed values 0,1,2, default is 0 meaning random test arg, 1 means test_arg = 1 2 means test arg = 2 \n"
	.text
	.align	2
	.p2align 3,,7
	.global	usage
	.type	usage, %function
usage:
.LFB73:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	mov	x2, 70
	mov	x1, 1
	mov	x29, sp
	str	x19, [sp, 16]
	.cfi_offset 19, -16
	adrp	x19, :got:stderr
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	ldr	x19, [x19, #:got_lo12:stderr]
	ldr	x3, [x19]
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 96
	mov	x1, 1
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 87
	mov	x1, 1
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 118
	mov	x1, 1
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 97
	mov	x1, 1
	adrp	x0, .LC6
	add	x0, x0, :lo12:.LC6
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 135
	mov	x1, 1
	adrp	x0, .LC7
	add	x0, x0, :lo12:.LC7
	bl	fwrite
	ldr	x3, [x19]
	mov	x2, 75
	mov	x1, 1
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	fwrite
	ldr	x3, [x19]
	adrp	x0, .LC9
	ldr	x19, [sp, 16]
	add	x0, x0, :lo12:.LC9
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 19
	.cfi_def_cfa_offset 0
	mov	x2, 109
	mov	x1, 1
	b	fwrite
	.cfi_endproc
.LFE73:
	.size	usage, .-usage
	.section	.rodata.str1.8
	.align	3
.LC12:
	.string	"the random conditional branch code requires at least 3 arguments  there were %d\n"
	.align	3
.LC13:
	.string	"insufficient invocation arguments"
	.align	3
.LC14:
	.string	"unknown option %c"
	.align	3
.LC15:
	.string	"i:r:l:m:T:C"
	.align	3
.LC16:
	.string	"invalid value for -T was %d must be 0 or 1 or 2\n"
	.align	3
.LC17:
	.string	"size_t in %zd bytes\n"
	.align	3
.LC18:
	.string	"failed to set affinity"
	.align	3
.LC19:
	.string	" process pinned to core %d\n"
	.align	3
.LC20:
	.string	" failed to malloc index array for line_count of %d\n"
	.align	3
.LC21:
	.string	"failed to malloc index"
	.align	3
.LC22:
	.string	" failed to malloc buf1 for len*sizeof(double) of %zd\n"
	.align	3
.LC23:
	.string	"failed to malloc buf1"
	.align	3
.LC24:
	.string	" failed to malloc buf2 for line_count*len*sizeof(double) of %zd\n"
	.align	3
.LC25:
	.string	"failed to malloc buf2"
	.align	3
.LC26:
	.string	"cannot set cpu run affinity"
	.align	3
.LC27:
	.string	" process pinned to core %d to run\n"
	.align	3
.LC28:
	.string	" calling conditional br loop %d times which loops  %d times on set of %d functions\n"
	.align	3
.LC29:
	.string	" done"
	.align	3
.LC30:
	.string	" run time = %zd\n"
	.align	3
.LC31:
	.string	" sum_run time = %zd\n"
	.align	3
.LC32:
	.string	" gettimeofday_run time = %zd\n"
	.align	3
.LC33:
	.string	"frequency = %zd\n"
	.align	3
.LC34:
	.string	" init_run = %zd, init_end = %zd, call_start = %zd\n"
	.align	3
.LC35:
	.string	"total = %lu\n"
	.align	3
.LC36:
	.string	" average cycles per iteration = %f\n"
	.section	.text.startup,"ax",@progbits
	.align	2
	.p2align 3,,7
	.global	main
	.type	main, %function
main:
.LFB74:
	.cfi_startproc
	sub	x12, sp, #3997696
	.cfi_def_cfa 12, 3997696
.LPSRL0:
	sub	sp, sp, 65536
	str	xzr, [sp, 1024]
	cmp	sp, x12
	b.ne	.LPSRL0
	.cfi_def_cfa_register 31
	sub	sp, sp, #2624
	.cfi_def_cfa_offset 4000320
	adrp	x4, :got:__stack_chk_guard
	adrp	x3, .LC10
	adrp	x2, .LC11
	add	x3, x3, :lo12:.LC10
	stp	x29, x30, [sp]
	add	x2, x2, :lo12:.LC11
	.cfi_offset 29, -4000320
	.cfi_offset 30, -4000312
	mov	x29, sp
	ldr	x8, [x4, #:got_lo12:__stack_chk_guard]
	stp	x21, x22, [sp, 32]
	.cfi_offset 21, -4000288
	.cfi_offset 22, -4000280
	mov	w21, w0
	add	x0, sp, 3997696
	stp	x23, x24, [sp, 48]
	add	x0, x0, 48
	.cfi_offset 23, -4000272
	.cfi_offset 24, -4000264
	mov	x24, x1
	ldrb	w1, [x3, 16]
	ldp	x6, x7, [x3]
	ldr	x3, [x8]
	str	x3, [x0, 2568]
	mov	x3,0
	ldrb	w0, [x2, 16]
	ldp	x4, x5, [x2]
	add	x2, sp, 3997696
	add	x2, x2, 2608
	stp	x6, x7, [x2, -40]
	add	x2, sp, 3997696
	add	x2, x2, 48
	strb	w1, [x2, 2536]
	add	x1, sp, 3997696
	add	x1, x1, 2608
	strb	w0, [x2, 2560]
	stp	x4, x5, [x1, -16]
#APP
// 53 "cond_br_driver_str.c" 1
	isb
// 0 "" 2
// 54 "cond_br_driver_str.c" 1
	mrs x0, cntvct_el0
// 0 "" 2
#NO_APP
	stp	x19, x20, [sp, 16]
	.cfi_offset 20, -4000296
	.cfi_offset 19, -4000304
	cmp	w21, 2
	stp	x25, x26, [sp, 64]
	.cfi_offset 26, -4000248
	.cfi_offset 25, -4000256
	stp	x27, x28, [sp, 80]
	.cfi_offset 28, -4000232
	.cfi_offset 27, -4000240
	str	x0, [sp, 136]
	ble	.L100
	adrp	x27, :got:optarg
	adrp	x23, .LC15
	add	x23, x23, :lo12:.LC15
	mov	w22, 0
	ldr	x28, [x27, #:got_lo12:optarg]
	mov	w19, 1
	str	x28, [sp, 120]
	.p2align 3,,7
.L39:
	mov	x2, x23
	mov	x1, x24
	mov	w0, w21
	bl	getopt
	cmn	w0, #1
	beq	.L101
.L48:
	cmp	w0, 108
	beq	.L40
	bgt	.L41
	cmp	w0, 84
	bne	.L102
	ldr	x0, [x27, #:got_lo12:optarg]
	mov	w2, 10
	mov	x1, 0
	ldr	x0, [x0]
	bl	strtol
	mov	w22, w0
	mov	x2, x23
	mov	x1, x24
	mov	w0, w21
	bl	getopt
	cmn	w0, #1
	bne	.L48
	.p2align 3,,7
.L101:
	adrp	x23, :got:stderr
	cmp	w22, 2
	ldr	x21, [x23, #:got_lo12:stderr]
	ldr	x0, [x21]
	bhi	.L103
	adrp	x2, .LC17
	add	x2, x2, :lo12:.LC17
	mov	x3, 8
	mov	w1, 1
	bl	__fprintf_chk
	mov	w0, w26
	bl	pin_cpu.constprop.0
	cmn	w0, #1
	beq	.L104
	ldr	x0, [x21]
	mov	w3, w26
	adrp	x2, .LC19
	add	x2, x2, :lo12:.LC19
	mov	w1, 1
	bl	__fprintf_chk
	sbfiz	x0, x20, 2, 32
	bl	malloc
	cbz	x0, .L51
	cmp	w20, 0
	ble	.L63
	cmp	w22, 1
	beq	.L55
	str	d8, [sp, 96]
	.cfi_offset 72, -4000224
	fmov	d8, 5.0e-1
	add	x28, sp, 192
	sub	w21, w20, #1
	add	x27, x28, 8
	add	x24, sp, 3997696
	add	x26, sp, 3997696
	bl	drand48
	add	x24, x24, 2592
	add	x21, x27, x21, uxtw 3
	add	x26, x26, 2568
	cmp	w22, 2
	bne	.L105
	.p2align 3,,7
.L56:
	str	x24, [x28]
.L59:
	cmp	x27, x21
	mov	x28, x27
	beq	.L98
.L106:
	bl	drand48
	add	x27, x27, 8
	cmp	w22, 2
	beq	.L56
.L105:
	cbnz	w22, .L59
	fcmpe	d0, d8
	bgt	.L56
	str	x26, [x28]
	cmp	x27, x21
	mov	x28, x27
	bne	.L106
.L98:
	ldr	d8, [sp, 96]
	.cfi_restore 72
.L63:
	mov	x5, 0
	mov	w4, -1
	mov	w3, 34
	mov	w2, 3
	mov	x1, 800
	mov	x0, 0
	bl	mmap
	mov	x21, x0
	cbz	x0, .L107
	mov	w22, 100
	mov	x5, 0
	mov	w4, -1
	mov	w3, 34
	mul	w22, w20, w22
	mov	w2, 3
	mov	x0, 0
	sbfiz	x26, x22, 3, 32
	mov	x1, x26
	bl	mmap
	str	x0, [sp, 120]
	cbz	x0, .L108
	mov	x5, 0
	mov	w4, -1
	mov	w3, 34
	mov	x1, x26
	mov	w2, 3
	mov	x0, 0
	bl	mmap
	str	x0, [sp, 128]
	mov	x2, 800
	mov	x0, x21
	mov	w1, 0
	bl	memset
	cmp	w22, 0
	ble	.L76
	sub	w0, w22, #1
	cmp	w0, 2
	bls	.L68
	ldr	x2, [sp, 120]
	lsr	w1, w22, 1
	fmov	v0.2d, 1.0e+0
	mov	x0, x2
	add	x2, x2, x1, uxtw 4
	.p2align 3,,7
.L69:
	str	q0, [x0], 16
	cmp	x0, x2
	bne	.L69
.L99:
	ldr	x0, [sp, 128]
	and	w2, w22, -2
	fmov	v0.2d, 2.0e+0
	add	x1, x0, x1, uxtw 4
.L74:
	str	q0, [x0], 16
	cmp	x0, x1
	bne	.L74
	mov	w0, w2
	cmp	w22, w2
	beq	.L76
.L73:
	add	w2, w0, 1
	sbfiz	x1, x0, 3, 32
	cmp	w22, w2
	fmov	d0, 2.0e+0
	ldr	x2, [sp, 128]
	str	d0, [x2, x1]
	ble	.L76
	add	x1, x2, x1
	add	w0, w0, 2
	cmp	w22, w0
	str	d0, [x1, 8]
	ble	.L76
	str	d0, [x1, 16]
.L76:
#APP
// 53 "cond_br_driver_str.c" 1
	isb
// 0 "" 2
// 54 "cond_br_driver_str.c" 1
	mrs x0, cntvct_el0
// 0 "" 2
#NO_APP
	str	x0, [sp, 144]
	mov	w0, w25
	bl	pin_cpu.constprop.0
	cmn	w0, #1
	beq	.L109
	mov	w2, w25
	adrp	x1, .LC27
	mov	w0, 1
	add	x1, x1, :lo12:.LC27
	bl	__printf_chk
	mov	w4, w20
	mov	w2, w19
	mov	w3, 100
	adrp	x1, .LC28
	mov	w0, 1
	add	x1, x1, :lo12:.LC28
	bl	__printf_chk
#APP
// 53 "cond_br_driver_str.c" 1
	isb
// 0 "" 2
// 54 "cond_br_driver_str.c" 1
	mrs x0, cntvct_el0
// 0 "" 2
#NO_APP
	mov	x1, 0
	str	x0, [sp, 152]
	add	x0, sp, 160
	bl	gettimeofday
	cmp	w19, 0
	ble	.L87
	sub	w0, w20, #1
	add	x25, sp, 3997696
	add	x0, x0, 1
	add	x25, x25, 2568
	ldr	x1, [sp, 120]
	add	x22, x0, x0, lsl 1
	mov	x27, 0
	mov	w23, 0
	add	x0, x0, x22, lsl 3
	add	x22, x1, x0, lsl 5
	.p2align 3,,7
.L79:
	cmp	w20, 0
	ble	.L82
	ldp	x28, x26, [sp, 120]
	add	x24, sp, 192
	.p2align 3,,7
.L80:
	ldr	x1, [x24], 8
	mov	x5, x26
	mov	x4, x28
	mov	x3, x21
	add	x28, x28, 800
	mov	x2, x25
	mov	w0, 100
	bl	triad
	add	x26, x26, 800
	cmp	x28, x22
	add	x27, x27, x0
	bne	.L80
.L82:
	add	w23, w23, 1
	cmp	w19, w23
	bne	.L79
.L78:
	adrp	x0, .LC29
	add	x0, x0, :lo12:.LC29
	bl	puts
#APP
// 53 "cond_br_driver_str.c" 1
	isb
// 0 "" 2
// 54 "cond_br_driver_str.c" 1
	mrs x20, cntvct_el0
// 0 "" 2
// 63 "cond_br_driver_str.c" 1
	isb
// 0 "" 2
// 64 "cond_br_driver_str.c" 1
	mrs x22, cntfrq_el0
// 0 "" 2
#NO_APP
	mov	x1, 0
	add	x0, sp, 176
	bl	gettimeofday
	ldp	x23, x0, [sp, 152]
	mov	x4, 16960
	ldp	x5, x21, [sp, 168]
	movk	x4, 0xf, lsl 16
	ldr	x3, [sp, 184]
	adrp	x1, .LC30
	sub	x20, x20, x23
	add	x1, x1, :lo12:.LC30
	mov	x2, x20
	sub	x3, x3, x5
	sub	x21, x21, x0
	mov	w0, 1
	madd	x21, x21, x4, x3
	bl	__printf_chk
	mov	x2, 0
	adrp	x1, .LC31
	mov	w0, 1
	add	x1, x1, :lo12:.LC31
	bl	__printf_chk
	mov	x2, x21
	adrp	x1, .LC32
	mov	w0, 1
	add	x1, x1, :lo12:.LC32
	bl	__printf_chk
	mov	x2, x22
	adrp	x1, .LC33
	mov	w0, 1
	add	x1, x1, :lo12:.LC33
	bl	__printf_chk
	ldp	x0, x3, [sp, 136]
	mov	x4, x23
	adrp	x1, .LC34
	add	x1, x1, :lo12:.LC34
	sub	x2, x3, x0
	mov	w0, 1
	bl	__printf_chk
	mov	x2, x27
	adrp	x1, .LC35
	mov	w0, 1
	add	x1, x1, :lo12:.LC35
	bl	__printf_chk
	scvtf	d1, w19
	mov	x0, 145839909502976
	movk	x0, 0x411e, lsl 48
	fmov	d2, x0
	ucvtf	d0, x20
	adrp	x1, .LC36
	mov	w0, 1
	add	x1, x1, :lo12:.LC36
	fmul	d1, d1, d2
	fdiv	d0, d0, d1
	bl	__printf_chk
	adrp	x0, :got:__stack_chk_guard
	add	x1, sp, 3997696
	add	x1, x1, 48
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x2, [x1, 2568]
	ldr	x0, [x0]
	eor	x0, x2, x0
	cbnz	x0, .L110
	ldp	x29, x30, [sp]
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	.cfi_remember_state
	.cfi_restore 20
	.cfi_restore 19
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	.cfi_restore 26
	.cfi_restore 25
	ldp	x27, x28, [sp, 80]
	.cfi_restore 28
	.cfi_restore 27
	.cfi_restore 29
	.cfi_restore 30
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	add	sp, sp, 2624
	.cfi_def_cfa_offset 3997696
	add	sp, sp, 3997696
	.cfi_def_cfa_offset 0
	ret
	.p2align 2,,3
.L41:
	.cfi_restore_state
	cmp	w0, 109
	bne	.L111
	ldr	x0, [sp, 120]
	mov	w2, 10
	mov	x1, 0
	ldr	x0, [x0]
	bl	strtol
	mov	w19, w0
	b	.L39
	.p2align 2,,3
.L102:
	cmp	w0, 105
	bne	.L44
	ldr	x0, [x27, #:got_lo12:optarg]
	mov	w2, 10
	mov	x1, 0
	ldr	x0, [x0]
	bl	strtol
	mov	w26, w0
	b	.L39
	.p2align 2,,3
.L111:
	cmp	w0, 114
	bne	.L44
	ldr	x0, [x27, #:got_lo12:optarg]
	mov	w2, 10
	mov	x1, 0
	ldr	x0, [x0]
	bl	strtol
	mov	w25, w0
	b	.L39
	.p2align 2,,3
.L40:
	ldr	x0, [x28]
	mov	w2, 10
	mov	x1, 0
	bl	strtol
	mov	w20, w0
	b	.L39
.L55:
	sub	w24, w20, #1
	add	x21, sp, 200
	add	x26, sp, 3997696
	add	x22, sp, 192
	add	x26, x26, 2568
	add	x24, x21, x24, uxtw 3
	b	.L62
	.p2align 2,,3
.L112:
	add	x21, x21, 8
.L62:
	bl	drand48
	str	x26, [x22]
	cmp	x21, x24
	mov	x22, x21
	bne	.L112
	b	.L63
.L87:
	mov	x27, 0
	b	.L78
.L68:
	ldr	x1, [sp, 120]
	fmov	d0, 1.0e+0
	cmp	w22, 1
	str	d0, [x1]
	beq	.L86
	ldr	x2, [sp, 120]
	mov	x1, 0
	fmov	d0, 1.0e+0
	add	x1, x2, x1
	mov	w2, 1
	add	w2, w2, w2
	cmp	w22, w2
	str	d0, [x1, 8]
	ble	.L72
	str	d0, [x1, 16]
.L72:
	cmp	w0, 2
	bls	.L86
	lsr	w1, w22, 1
	b	.L99
.L86:
	mov	w0, 0
	b	.L73
.L44:
	mov	w2, w0
	adrp	x1, .LC14
	mov	w0, 1
	add	x1, x1, :lo12:.LC14
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	err
.L104:
	.cfi_restore_state
	adrp	x1, .LC18
	mov	w0, 1
	add	x1, x1, :lo12:.LC18
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	err
.L103:
	.cfi_restore_state
	mov	w3, w22
	adrp	x2, .LC16
	add	x2, x2, :lo12:.LC16
	mov	w1, 1
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	__fprintf_chk
	mov	w0, 1
	bl	exit
.L51:
	.cfi_restore_state
	ldr	x0, [x21]
	mov	w3, w20
	adrp	x2, .LC20
	add	x2, x2, :lo12:.LC20
	mov	w1, 1
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	__fprintf_chk
	adrp	x1, .LC21
	mov	w0, 1
	add	x1, x1, :lo12:.LC21
	bl	err
.L109:
	.cfi_restore_state
	adrp	x1, .LC26
	mov	w0, 1
	add	x1, x1, :lo12:.LC26
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	err
.L108:
	.cfi_restore_state
	ldr	x23, [x23, #:got_lo12:stderr]
	mov	x3, x26
	adrp	x2, .LC24
	add	x2, x2, :lo12:.LC24
	mov	w1, 1
	ldr	x0, [x23]
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	__fprintf_chk
	adrp	x1, .LC25
	mov	w0, 1
	add	x1, x1, :lo12:.LC25
	bl	err
.L107:
	.cfi_restore_state
	ldr	x23, [x23, #:got_lo12:stderr]
	adrp	x2, .LC22
	add	x2, x2, :lo12:.LC22
	mov	x3, 800
	mov	w1, 1
	ldr	x0, [x23]
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	__fprintf_chk
	adrp	x1, .LC23
	mov	w0, 1
	add	x1, x1, :lo12:.LC23
	bl	err
.L100:
	.cfi_restore_state
	adrp	x0, :got:stderr
	mov	w3, w21
	adrp	x2, .LC12
	add	x2, x2, :lo12:.LC12
	ldr	x0, [x0, #:got_lo12:stderr]
	mov	w1, 1
	ldr	x0, [x0]
	str	d8, [sp, 96]
	.cfi_remember_state
	.cfi_offset 72, -4000224
	bl	__fprintf_chk
	bl	usage
	adrp	x1, .LC13
	mov	w0, 1
	add	x1, x1, :lo12:.LC13
	bl	err
.L110:
	.cfi_restore_state
	str	d8, [sp, 96]
	.cfi_offset 72, -4000224
	bl	__stack_chk_fail
	.cfi_endproc
.LFE74:
	.size	main, .-main
	.section	.rodata.str1.8
	.align	3
.LC10:
	.string	"this_is_string_1"
	.align	3
.LC11:
	.string	"this_is_string_2"
	.section	.text.startup
	.section	.rodata.cst16,"aM",@progbits,16
	.align	4
.LC1:
	.word	0
	.word	1
	.word	2
	.word	3
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
