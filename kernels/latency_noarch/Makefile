walker:	linked_list.o walker_tsc.c
	gcc -O0 walker_tsc.c linked_list.o -o walker

linked_list.o:
	gcc -c -mcpu=native -O3 linked_list.c

.PHONY: clean
clean:
	rm *.o walker


