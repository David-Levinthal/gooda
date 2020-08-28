	.file	"triad.c"
	.text
	.p2align 4,,15
	.globl	triad
	.type	triad, @function
triad:
.LFB52:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rsi, %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	movq	%r8, %rbp
	movq	%r9, %r12
	call	dumb_compare@PLT
	testl	%eax, %eax
	movq	%r12, %rdx
	movq	%rbp, %rsi
	movq	%rbx, %rdi
	je	.L6
	call	FOO2@PLT
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	cltq
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L6:
	.cfi_restore_state
	call	FOO1@PLT
	popq	%rbx
	.cfi_def_cfa_offset 24
	cltq
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE52:
	.size	triad, .-triad
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
