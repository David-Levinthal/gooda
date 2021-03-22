	.file	"FOO1.c"
	.text
	.p2align 4,,15
	.globl	FOO1
	.type	FOO1, @function
FOO1:
.LFB52:
	.cfi_startproc
	movupd	256(%rsi), %xmm15
	movl	$16, %eax
	movups	%xmm15, 256(%rdi)
	movupd	272(%rsi), %xmm15
	movups	%xmm15, 272(%rdi)
	movupd	288(%rsi), %xmm15
	movups	%xmm15, 288(%rdi)
	movupd	304(%rsi), %xmm15
	movups	%xmm15, 304(%rdi)
	movupd	320(%rsi), %xmm15
	movups	%xmm15, 320(%rdi)
	movupd	336(%rsi), %xmm15
	movups	%xmm15, 336(%rdi)
	movupd	352(%rsi), %xmm15
	movups	%xmm15, 352(%rdi)
	movupd	368(%rsi), %xmm15
	movupd	16(%rsi), %xmm14
	movupd	32(%rsi), %xmm13
	movupd	48(%rsi), %xmm12
	movupd	64(%rsi), %xmm11
	movupd	80(%rsi), %xmm10
	movupd	96(%rsi), %xmm9
	movupd	112(%rsi), %xmm8
	movupd	128(%rsi), %xmm7
	movupd	144(%rsi), %xmm6
	movupd	160(%rsi), %xmm5
	movupd	176(%rsi), %xmm4
	movupd	192(%rsi), %xmm3
	movupd	208(%rsi), %xmm2
	movupd	224(%rsi), %xmm1
	movupd	240(%rsi), %xmm0
	movups	%xmm15, 368(%rdi)
	movupd	(%rsi), %xmm15
	movups	%xmm15, (%rdi)
	movups	%xmm14, 16(%rdi)
	movups	%xmm13, 32(%rdi)
	movups	%xmm12, 48(%rdi)
	movups	%xmm11, 64(%rdi)
	movups	%xmm10, 80(%rdi)
	movups	%xmm9, 96(%rdi)
	movups	%xmm8, 112(%rdi)
	movups	%xmm7, 128(%rdi)
	movups	%xmm6, 144(%rdi)
	movups	%xmm5, 160(%rdi)
	movups	%xmm4, 176(%rdi)
	movups	%xmm3, 192(%rdi)
	movups	%xmm2, 208(%rdi)
	movups	%xmm1, 224(%rdi)
	movups	%xmm0, 240(%rdi)
	ret
	.cfi_endproc
.LFE52:
	.size	FOO1, .-FOO1
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
