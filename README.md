# buffered_queue

## Introduction

buffered_queue is a ring queue implementation in C which useses bulk-write and
per-thread buffering techniques to minimize the thread synchronization overhead
which in turn allows to efficiently scale even relatively small and fast
computations to multiple cores.

## Principle of operation

buffered_queue employs two major techniques to reduce the inter-thread
synchronization overhead to a minimum:

* The "write" or "push" operation allows the producer(s) to write multiple
  elements per call (i.e. per mutex-acquire).

* The "read" or "pop" operation, transparently for the user of the queue,
  uses a per consumer thread local storage to buffer a configurable number
  of elements locally, so that the next queue reads do not have to acquire
  the mutex of the queue, thus avoiding mutex contention, thus avoiding
  sleep via system calls, thus avoiding the syncrhonization overhead and
  maximizing the performance.

## Performance

A benchmark is included together with this code, you can run it by executing
`make run-benchmark`.

The benchmark outputs 3 results:

* 'Single thread' - this is how long it takes for a single-threaded version of
   the algorithm to complete;

* 'Simple queue read' - this is how long it takes for the algorithm to complete
  with a naive implementation of the queue read, where it acquires the mutex
  every time;

* 'Buffered queue read' - this is how long it takes for the algorithm to complete
   when using the buffered_queue.

Result on my computer:

```
CPU: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz

Single thread:

real	0m9.455s
user	0m9.442s
sys	0m0.001s

Simple queue read:

real	0m57.016s
user	1m21.308s
sys	1m2.559s

Buffered queue read:

real	0m5.668s
user	0m12.292s
sys	0m0.931s
```
