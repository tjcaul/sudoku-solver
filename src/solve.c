#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "solve.h"
#include "check.h"
#include "io.h"
#include "debug.h"
#include "thread.h"

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
