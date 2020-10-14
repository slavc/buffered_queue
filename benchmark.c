#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffered_queue.h"

int g_num_iters = 100000000;
int g_num_producers = 1;
int g_num_consumers = 2;
uintptr_t (*g_bq_read_func)(struct bq *bq);

double my_sqrt(double f)
{
	double a, b, c, d;
	a = 0;
	b = f;
	for (int i = 0; i < 30; i++) {
		c = a + (b - a) / 2.0;
		d = c*c;
		if (d > f) {
			b = c;
		} else if (d < f) {
			a = c;
		} else {
			return c;
		}
	}
	return c;
}

uint64_t gen_number(void)
{
	static uint64_t a = 0, b = 1, c;
	c = a + b;
	a = b;
	b = c;
	if (c > 1000) {
		a = 0;
		b = 1;
	}
	return c;
}

int producer(void *arg)
{
	struct bq *bq = arg;
	uintptr_t values[bq->buf_cap];
	uint64_t i;

	for (i = 0;; i++) {
		values[i & (bq->buf_cap-1)] = gen_number();
		if ((i & (bq->buf_cap-1)) == (bq->buf_cap-1)) {
			bq_write(bq, values, bq->buf_cap);
		}
	}

	return 0;
}

int consumer(void *arg)
{
	struct bq *bq = arg;

	double f, sum = 0.0;
	for (int n = g_num_iters / g_num_consumers; n > 0; n--) {
		f = g_bq_read_func(bq);
		sum += my_sqrt(f);
	}
	printf("%f\n", sum);

	return 0;
}

void single_thread(void)
{
	double f, sum = 0.0;
	for (int i = 0; i < g_num_iters; i++) {
		f = gen_number();
		sum += my_sqrt(f);

	}
	printf("%f\n", sum);
}

uintptr_t bq_read_simple(struct bq *bq)
{
	uintptr_t	elem;

	mtx_lock(&bq->mtx);
	while (bq->w == bq->r) {
		cnd_wait(&bq->r_cnd, &bq->mtx);
	}
	elem = bq->elems[bq->r++ & (bq->cap-1)];
	if (bq->w - bq->r > 0) {
		cnd_signal(&bq->r_cnd);
	}
	cnd_signal(&bq->w_cnd);
	mtx_unlock(&bq->mtx);
	return elem;
}

int main(int argc, char **argv)
{
	g_bq_read_func = bq_read;
	if (argc > 1) {
		if (strcmp(argv[1], "single_thread") == 0) {
			single_thread();
			return 0;
		} else if (strcmp(argv[1], "simple_queue_read") == 0) {
			g_bq_read_func = bq_read_simple;
		}
	}

	size_t buf_cap = 64 << g_num_consumers;
	size_t cap = g_num_consumers * buf_cap;

	struct bq *bq = bq_new(cap, buf_cap);

	thrd_t threads[g_num_producers+g_num_consumers];

	for (int i = 0; i < g_num_producers; i++) {
		thrd_create(&threads[i], producer, bq);
	}

	for (int i = 0; i < g_num_consumers; i++) {
		thrd_create(&threads[g_num_producers+i], consumer, bq);
	}

	for (int i = 0; i < g_num_consumers; i++) {
		thrd_join(threads[g_num_producers+i], NULL);
	}

	return 0;
}
