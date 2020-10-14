CFLAGS += -g3 -Wall -Werror -Wextra -pedantic -std=c11 -O2 -I.

.PHONY: all clean run-benchmark

all: libbq.so libbq.a benchmark

clean:
	rm -f *.o *.so *.a benchmark

run-benchmark: benchmark
	@echo CPU: `grep 'model name' /proc/cpuinfo | head -1 | awk -F : '{ print $$2 }'`
	@echo Kernel command line: `cat /proc/cmdline`
	@echo
	@echo Single thread:
	@time ./benchmark single_thread >/dev/null
	@echo
	@echo Simple queue read:
	@time ./benchmark simple_queue_read >/dev/null
	@echo
	@echo Buffered queue read:
	@time ./benchmark >/dev/null

libbq.a: buffered_queue.h buffered_queue.c
	gcc $(CFLAGS) -lpthread -c -o buffered_queue.o buffered_queue.c
	ar rcs $@ buffered_queue.o

libbq.so: buffered_queue.h buffered_queue.c
	gcc $(CFLAGS) -lpthread -fPIC -shared -o $@ buffered_queue.c

benchmark: buffered_queue.h buffered_queue.c benchmark.c libbq.a
	gcc $(CFLAGS) -lpthread -o $@ benchmark.c libbq.a

