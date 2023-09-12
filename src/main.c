#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "check.h"
#include "io.h"
#include "solve.h"
#include "debug.h"

/* compile defines:
 * DEBUG	print questionably useful debug messages
 * PROGRESS	show progress bar while solving. Set PROGRESS=delay
 * 			to add an artifical delay to the progress bar.
 */

void usage(char **argv)
{
	printf("Usage: %s [puzzle-file]\n", argv[0]);
}

int main(int argc, char **argv)
{
	FILE *file;
	int *board;

	if (argc == 1) {
		file = stdin;
	} else if (argc == 2) {
		file = fopen(argv[1], "r");
		if (!file) {
			perror("fopen");
			exit(1);
		}
	} else {
		usage(argv);
		exit(1);
	}

	board = malloc(9 * 9 * sizeof(*board));
	if (!board) {
		perror("malloc");
		exit(1);
	}

	load_board(board, file);
	print_board(board);
	int *solved = solve(board);
	putchar('\n');
	if (solved) {
		printf("Solved!\n");
		print_board(solved);
		free(solved);
	} else {
		printf("Impossible.\n");
	}
	return 0;
}
