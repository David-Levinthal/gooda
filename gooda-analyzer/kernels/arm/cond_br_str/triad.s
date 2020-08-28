	.arch armv8-a
	.file	"triad.c"
	.text
	.align	2
	.p2align 3,,7
	.global	triad
	.type	triad, %function
triad:
.LFB50:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	mov	x0, x1
	mov	x1, x2
	mov	x29, sp
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -32
	.cfi_offset 20, -24
	mov	x20, x4
	mov	x19, x3
	str	x21, [sp, 32]
	.cfi_offset 21, -16
	mov	x21, x5
	bl	dumb_compare
	mov	x2, x21
	mov	x1, x20
	cbnz	w0, .L2
	mov	x0, x19
	bl	FOO1
	sxtw	x0, w0
	ldp	x19, x20, [sp, 16]
	ldr	x21, [sp, 32]
	ldp	x29, x30, [sp], 48
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 21
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
	.p2align 2,,3
.L2:
	.cfi_restore_state
	mov	x0, x19
	bl	FOO2
	sxtw	x0, w0
	ldp	x19, x20, [sp, 16]
	ldr	x21, [sp, 32]
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 21
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE50:
	.size	triad, .-triad
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
