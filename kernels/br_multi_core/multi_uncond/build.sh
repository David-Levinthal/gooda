#! /bin/bash
gcc -O0 uncond_rdm_jmp64.c -o generator64
	./generator64 $1
	gcc -O0 driver.c FOO_loop.c -o rjmp64_$1

