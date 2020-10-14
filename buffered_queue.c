#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "buffered_queue.h"

static size_t make_pow2(size_t n)
{
	size_t i;
	for (i = 2; i < n && i != 0; i <<= 1) {
		/* empty */;
	}
	if (i == 0) {
		return 0x400;
	}
	return i;
}

struct bq *bq_new(size_t cap, size_t buf_cap)
{
	return bq_init(malloc(sizeof(struct bq) + cap*sizeof(uintptr_t)),
								cap, buf_cap);
}

struct bq *bq_init(struct bq *bq, size_t cap, size_t buf_cap)
{
	memset(bq, 0, sizeof(*bq));
	mtx_init(&bq->mtx, mtx_plain);
	cnd_init(&bq->r_cnd);
	cnd_init(&bq->w_cnd);
	bq->buf_cap = make_pow2(buf_cap);
	bq->cap = make_pow2(cap);
	return bq;
}

void bq_write(struct bq *bq, uintptr_t *values, size_t n)
{
	mtx_lock(&bq->mtx);
	while (n > 0) {
		while (bq->w - bq->r == bq->cap) {
			cnd_wait(&bq->w_cnd, &bq->mtx);
		}
		while (n > 0 && bq->w - bq->r < bq->cap) {
			bq->elems[bq->w++ & (bq->cap-1)] = *values++;
			n--;
		}
	}
	if (bq->w - bq->r < bq->cap) {
		cnd_signal(&bq->w_cnd);
	}
	cnd_signal(&bq->r_cnd);
	mtx_unlock(&bq->mtx);
}

uintptr_t bq_read(struct bq *bq)
{
	struct bq_buf	*buf;

	buf = tss_get(bq->buf);
	if (buf == NULL) {
		tss_create(&bq->buf, free);
		buf = malloc(sizeof(struct bq_buf)
				+ bq->buf_cap * sizeof(uintptr_t));
		buf->r = buf->n = 0;
		buf->cap = bq->buf_cap;
		tss_set(bq->buf, buf);
	}

	if (buf->r == buf->n) {
		buf->r = buf->n = 0;
		mtx_lock(&bq->mtx);
		while (bq->w == bq->r) {
			cnd_wait(&bq->r_cnd, &bq->mtx);
		}
		while (buf->n < buf->cap && buf->n < bq->w - bq->r) {
			buf->elems[buf->n++] = bq->elems[bq->r++ & (bq->cap-1)];
		}
		if (bq->w - bq->r > 0) {
			cnd_signal(&bq->r_cnd);
		}
		cnd_signal(&bq->w_cnd);
		mtx_unlock(&bq->mtx);
	}
	return buf->elems[buf->r++];
}
