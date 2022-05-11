all:	walker_low_util walker_hi

walker_low_util:	linked_list.o walker_low_util.o
	gcc -O0 walker_low_util.o linked_list.o -o walker_low_util

walker_hi:	linked_list.o walker_hi_rate.o
	gcc -O0 walker_hi_rate.o linked_list.o -o walker_hi_rate

walker_low_util.o:
	gcc -O0 -c walker_low_util.c

walker_hi_rate.o:
	gcc -O0 -c walker_hi_rate.c

linked_list.o:
	gcc -c -mcpu=native -O3 linked_list.c

.PHONY: clean
clean:
	rm *.o walker*.o walker_low_util walker_hi_rate


