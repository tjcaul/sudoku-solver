#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include "thread.h"
#include "debug.h"
#include "solve.h"

#ifndef THREADS
#define THREADS 5 //Set to 0 to disable multi-threading. Default found experimentally.
#endif

int num_threads = 0;
pthread_mutex_t num_threads_mutex;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

void mutex_init(void)
{
	debug("mutex_init()\n");
	pthread_mutex_init(&num_threads_mutex, NULL);
}

int threadpool_spawn(pthread_t *threadpool, int *board)
{
	pthread_mutex_lock(&num_threads_mutex);
	if (num_threads >= THREADS) {
		pthread_mutex_unlock(&num_threads_mutex);
		return 1;
	}
	for (int i = 0; i < THREADPOOL_SZ; ++i) {
		if (threadpool[i] == NULL) {
			if ((errno = pthread_create
			    (&threadpool[i], NULL, solve, (void *)board))) {
				perror("pthread_create");
				pthread_mutex_unlock(&num_threads_mutex);
				return 1;
			} else {
				++num_threads;
				pthread_mutex_unlock(&num_threads_mutex);
				debug("spawned thread #%d, total now %d\n", i,
				      num_threads);
				return 0;
			}
		}
	}
	debug("threadpool_spawn: no space in threadpool of size %d\n",
	      THREADPOOL_SZ);
	pthread_mutex_unlock(&num_threads_mutex);
	return 1;
}

int *threadpool_join(pthread_t *threadpool)
{
	void *best_out = NULL;
	for (int i = 0; i < THREADPOOL_SZ; ++i) {
		if (threadpool[i] != NULL) {
			void *out;
			if ((errno = pthread_join(threadpool[i], &out))) {
				perror("pthread_join");
				exit(1);
			} else {
				pthread_mutex_lock(&num_threads_mutex);
				--num_threads;
				debug("thread #%d exited with out=%p, total now %d\n", i, out, num_threads);
				pthread_mutex_unlock(&num_threads_mutex);
				threadpool[i] = NULL;
				if (out && !best_out)
					best_out = out;
			}
		}
	}
	return best_out;
}

void threadpool_cancel(void *arg)
{
	pthread_t *threadpool = arg;
	//debug("threadpool: %p %p %p %p %p %p %p %p %p\n", threadpool[0],  threadpool[1], threadpool[2], threadpool[3], threadpool[4], threadpool[5], threadpool[6], threadpool[7], threadpool[8]);
	for (int i = 0; i < THREADPOOL_SZ; ++i) {
		if (threadpool[i] != NULL) {
			debug("cancelling #%d\n", i);
			if ((errno = pthread_detach(threadpool[i]))) {
				perror("pthread_detach");
				exit(1);
			}
			if ((errno = pthread_cancel(threadpool[i]))) {
				perror("pthread_cancel");
				exit(1);
			}
			pthread_mutex_lock(&num_threads_mutex);
			debug("thread cancelled, total now %d\n",
			      num_threads);
			--num_threads;
			pthread_mutex_unlock(&num_threads_mutex);
			threadpool[i] = NULL;
		}
	}
}
