pagesize := $(shell getconf PAGESIZE)

run: walker
	perf stat -C 1 -e instructions,cycles ./walker -p $(pagesize) -i0 -r1 -l4096 -s1 -S256 -I 10

walker: linked_list.o walker.c
	gcc -O0 walker.c linked_list.o -o walker

linked_list.o:
	gcc -c -mcpu=native -O3 linked_list.c

.PHONY: clean
clean:
	rm *.o walker

