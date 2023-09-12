#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "solve.h"
#include "check.h"
#include "io.h"
#include "debug.h"

#ifndef THREADS
#define THREADS 5 //Set to 0 to disable multi-threading. Default found experimentally.
#endif
#define THREADPOOL_SZ 9	//Number of child threads a thread can have; one for each possible value of cell

int num_threads = 0;
pthread_mutex_t num_threads_mutex;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void mutex_init(void)
{
	debug("mutex_init()\n");
	pthread_mutex_init(&num_threads_mutex, NULL);
}

static int threadpool_spawn(pthread_t *threadpool, int *board)
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

static int *threadpool_join(pthread_t *threadpool)
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

static void threadpool_cancel(void *arg)
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

void *solve(void *arg)
{
	pthread_t threadpool[THREADPOOL_SZ];
	int *board = (int *)arg;
	size_t board_sz = 9 * 9 * sizeof(*board);
	void *out;

	pthread_cleanup_push(free, board);
	pthread_once(&once_control, mutex_init);
	memset(threadpool, 0, THREADPOOL_SZ * sizeof(*threadpool));

#ifdef PROGRESS
	putchar('.');
	fflush(stdout);
#if PROGRESS == delay
	usleep(10000);
#endif
#endif

	int cell = 0;
	while (board[cell] != 0) {
		++cell;
		if (cell >= 81) {
			debug("Reached maximum depth, returning\n");
			return (void *)board;
		}
	}

#ifdef DEBUG
	//debug("Solving cell %d on:\n", cell);
	//print_board(board);
#endif

	for (int val = 1; val <= 9; ++val) {
		int *new_board = malloc(board_sz); //new_board is malloc'd by the parent but freed by the child
		if (!new_board)
			perror("new_board = malloc()");
		memcpy(new_board, board, board_sz);
		new_board[cell] = val;
		if (check_board(new_board, cell)) {
			if (threadpool_spawn(threadpool, new_board) != 0) {
				out = solve(new_board);
				if (out) {
					threadpool_cancel((void *) threadpool);
					return out;
				}
			}
		} else {
			free(new_board);
		}
	}
	out = threadpool_join(threadpool);

	//debug("Backing out\n");
#ifdef PROGRESS
	printf("\b \b"); //Overwrite with space, then back up
	fflush(stdout);
#endif

	pthread_cleanup_pop(1);
	return out;
}
