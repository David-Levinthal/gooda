	.arch armv8-a
	.file	"dumb_compare.c"
	.text
	.align	2
	.p2align 3,,7
	.global	dumb_compare
	.type	dumb_compare, %function
dumb_compare:
.LFB50:
	.cfi_startproc
	ldrb	w2, [x1]
	ldrb	w4, [x0]
	ldrb	w5, [x1, 1]
	ldrb	w3, [x0, 1]
	sub	w4, w4, w2
	ldrb	w6, [x1, 2]
	ldrb	w2, [x0, 2]
	sub	w3, w3, w5
	ldrb	w7, [x1, 3]
	add	w3, w3, w4
	ldrb	w5, [x0, 3]
	sub	w2, w2, w6
	ldrb	w4, [x0, 4]
	add	w2, w2, w3
	ldrb	w6, [x1, 4]
	sub	w5, w5, w7
	ldrb	w3, [x0, 5]
	add	w5, w5, w2
	ldrb	w7, [x1, 5]
	sub	w4, w4, w6
	ldrb	w2, [x0, 6]
	add	w4, w4, w5
	ldrb	w6, [x1, 6]
	sub	w3, w3, w7
	ldrb	w5, [x0, 7]
	add	w3, w3, w4
	ldrb	w7, [x1, 7]
	sub	w2, w2, w6
	ldrb	w4, [x0, 8]
	add	w2, w2, w3
	ldrb	w6, [x1, 8]
	sub	w5, w5, w7
	ldrb	w3, [x0, 9]
	add	w5, w5, w2
	ldrb	w7, [x1, 9]
	sub	w4, w4, w6
	ldrb	w2, [x0, 10]
	add	w4, w4, w5
	ldrb	w6, [x1, 10]
	sub	w3, w3, w7
	ldrb	w5, [x0, 11]
	add	w3, w3, w4
	ldrb	w7, [x1, 11]
	sub	w2, w2, w6
	ldrb	w4, [x0, 12]
	add	w2, w2, w3
	ldrb	w6, [x1, 12]
	sub	w5, w5, w7
	ldrb	w3, [x0, 13]
	add	w5, w5, w2
	ldrb	w7, [x1, 13]
	sub	w4, w4, w6
	ldrb	w2, [x0, 14]
	add	w4, w4, w5
	sub	w3, w3, w7
	ldrb	w6, [x1, 14]
	ldrb	w5, [x0, 15]
	add	w0, w3, w4
	ldrb	w3, [x1, 15]
	sub	w1, w2, w6
	add	w1, w1, w0
	sub	w0, w5, w3
	add	w0, w0, w1
	ret
	.cfi_endproc
.LFE50:
	.size	dumb_compare, .-dumb_compare
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
