	.file	"dumb_compare.c"
	.text
	.p2align 4,,15
	.globl	dumb_compare
	.type	dumb_compare, @function
dumb_compare:
.LFB52:
	.cfi_startproc
	movsbl	(%rsi), %eax
	movsbl	(%rdi), %edx
	movsbl	1(%rsi), %ecx
	subl	%eax, %edx
	movl	%edx, %eax
	movsbl	1(%rdi), %edx
	subl	%ecx, %edx
	movsbl	2(%rsi), %ecx
	addl	%eax, %edx
	movsbl	2(%rdi), %eax
	subl	%ecx, %eax
	movsbl	3(%rsi), %ecx
	addl	%eax, %edx
	movsbl	3(%rdi), %eax
	subl	%ecx, %eax
	movsbl	4(%rsi), %ecx
	addl	%edx, %eax
	movsbl	4(%rdi), %edx
	subl	%ecx, %edx
	movsbl	5(%rsi), %ecx
	addl	%edx, %eax
	movsbl	5(%rdi), %edx
	subl	%ecx, %edx
	movsbl	6(%rsi), %ecx
	addl	%eax, %edx
	movsbl	6(%rdi), %eax
	subl	%ecx, %eax
	movsbl	7(%rsi), %ecx
	addl	%eax, %edx
	movsbl	7(%rdi), %eax
	subl	%ecx, %eax
	movsbl	8(%rsi), %ecx
	addl	%edx, %eax
	movsbl	8(%rdi), %edx
	subl	%ecx, %edx
	movsbl	9(%rsi), %ecx
	addl	%edx, %eax
	movsbl	9(%rdi), %edx
	subl	%ecx, %edx
	movsbl	10(%rsi), %ecx
	addl	%eax, %edx
	movsbl	10(%rdi), %eax
	subl	%ecx, %eax
	movsbl	11(%rsi), %ecx
	addl	%eax, %edx
	movsbl	11(%rdi), %eax
	subl	%ecx, %eax
	movsbl	12(%rsi), %ecx
	addl	%edx, %eax
	movsbl	12(%rdi), %edx
	subl	%ecx, %edx
	movsbl	13(%rsi), %ecx
	addl	%edx, %eax
	movsbl	13(%rdi), %edx
	subl	%ecx, %edx
	movsbl	14(%rsi), %ecx
	addl	%eax, %edx
	movsbl	14(%rdi), %eax
	subl	%ecx, %eax
	movsbl	15(%rsi), %ecx
	addl	%eax, %edx
	movsbl	15(%rdi), %eax
	subl	%ecx, %eax
	addl	%edx, %eax
	ret
	.cfi_endproc
.LFE52:
	.size	dumb_compare, .-dumb_compare
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
