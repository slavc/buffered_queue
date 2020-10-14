#ifndef BUFFERED_QUEUE_H
#define BUFFERED_QUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <threads.h>

struct bq {
	uint64_t	r;
	uint64_t	w;
	mtx_t		mtx;
	cnd_t		r_cnd;
	cnd_t		w_cnd;
	size_t		buf_cap;
	tss_t		buf;
	size_t		cap;
	uintptr_t	elems[];
};

struct bq_buf {
	uint64_t	r;
	uint64_t	n;
	size_t		cap;
	uintptr_t	elems[];
};

struct bq *bq_new(size_t cap, size_t buf_cap);
struct bq *bq_init(struct bq *bq, size_t cap, size_t buf_cap);
void bq_write(struct bq *bq, uintptr_t *values, size_t n);
uintptr_t bq_read(struct bq *bq);

#endif
