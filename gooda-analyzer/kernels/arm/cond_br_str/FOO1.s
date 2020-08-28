	.arch armv8-a
	.file	"FOO1.c"
	.text
	.align	2
	.p2align 3,,7
	.global	FOO1
	.type	FOO1, %function
FOO1:
.LFB50:
	.cfi_startproc
	ldp	q1, q0, [x1]
	mov	x2, x0
	ldr	q2, [x1, 32]
	mov	w0, 16
	stp	q1, q0, [x2]
	ldp	q1, q0, [x1, 48]
	str	q2, [x2, 32]
	ldr	q2, [x1, 80]
	stp	q1, q0, [x2, 48]
	ldp	q1, q0, [x1, 96]
	str	q2, [x2, 80]
	ldp	q23, q22, [x1, 128]
	stp	q1, q0, [x2, 96]
	ldp	q21, q20, [x1, 160]
	ldp	q19, q18, [x1, 192]
	ldp	q17, q16, [x1, 224]
	ldp	q7, q6, [x1, 256]
	ldp	q5, q4, [x1, 288]
	ldr	q3, [x1, 320]
	ldr	q2, [x1, 336]
	ldp	q1, q0, [x1, 352]
	stp	q23, q22, [x2, 128]
	stp	q21, q20, [x2, 160]
	stp	q19, q18, [x2, 192]
	stp	q17, q16, [x2, 224]
	stp	q7, q6, [x2, 256]
	stp	q5, q4, [x2, 288]
	stp	q3, q2, [x2, 320]
	stp	q1, q0, [x2, 352]
	ret
	.cfi_endproc
.LFE50:
	.size	FOO1, .-FOO1
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
