#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* compile defines:
 * DEBUG	print questionably useful debug messages
 * PROGRESS	show progress bar while solving. Set PROGRESS=delay
 * 			to add an artifical delay to the progress bar.
 */

#ifdef DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif

void usage (char **argv)
{
	printf("Usage: %s [puzzle-file]\n", argv[0]);
}

int load_board (int *board, FILE *file)
{
	for (int i = 0; i < 9*9; ) {
		char c = getc(file);
		if (c == EOF) {
			printf("load_board: hit EOF\n");
			return 1;
		}
		if (strchr("123456789", c))
			board[i++] = c - '0';
		else if (c == ' ')
			board[i++] = 0;
	}
	return 0;
}

void print_board (int *board)
{
	puts(".-----.-----.-----.");
	for (int i = 0; i < 9; ++i) {
		putchar('|');
		for (int j = 0; j < 9; ++j) {
			char c = board[i*9+j] + '0';
			if (c == '0')
				c = ' ';
			putchar(c);
			if (j%3 == 2)
				putchar('|');
			else
				putchar(' ');
		}
		putchar('\n');
		if (i == 8)
			puts("'-----'-----'-----'");
		else if (i%3 == 2)
			puts("|-----+-----+-----|");
	}
}

int check_row (int *board, int row)
{
	int full[9];
	memset(full, 0, 9*sizeof(*full));
	for (int i = 0; i < 9; ++i) {
		int val = board[row*9 + i];
		if (val == 0) 
			continue;
		++full[val-1];
		if (full[val-1] > 1)
			return 0;
	}
	return 1;
}

int check_col (int *board, int col)
{
	int full[9];
	memset(full, 0, 9*sizeof(*full));
	for (int i = 0; i < 9; ++i) {
		int val = board[i*9 + col];
		if (val == 0) 
			continue;
		++full[val-1];
		if (full[val-1] > 1)
			return 0;
	}
	return 1;
}

int check_box (int *board, int row, int col)
{
	row = (row / 3) * 3;
	col = (col / 3) * 3;
	int full[9];
	memset(full, 0, 9*sizeof(*full));
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int val = board[row * 9 + col + i*9 + j];
			if (val == 0) 
				continue;
			++full[val-1];
			if (full[val-1] > 1)
				return 0;
		}
	}
	return 1;
}

int check_board (int *board, int cell)
{
	int row = cell / 9;
	int col = cell % 9;
	int valid = check_row(board, row) && check_col(board, col) && check_box(board, row, col);
	//debug("Checking row %d, col %d: %s\n", row, col, valid?"valid":"invalid");
	return valid;
}

int *solve (int *board)
{
	#ifdef PROGRESS
		putchar('.');
		fflush(stdout);
		#if PROGRESS == delay
			usleep(10000);
		#endif
	#endif
	size_t board_sz = 9*9*sizeof(*board);
	int *new_board = malloc(board_sz);
	memcpy(new_board, board, board_sz);
	int cell = 0;
	while (new_board[cell] != 0) {
		++cell;
		if (cell >= 81) {
			debug("Reached maximum depth, returning\n");
			return new_board;
		}
	}

	debug("Solving cell %d on:\n", cell);
	#ifdef DEBUG
		print_board(board);
	#endif

	for (int val = 1; val <= 9; ++val) {
		new_board[cell] = val;
		if (check_board(new_board, cell)) {
			int *out = solve(new_board);
			if (out)
				return out;
		}
	}
	debug("Backing out\n");
	#ifdef PROGRESS
		printf("\b \b"); //Overwrite with space, then back up
		fflush(stdout);
	#endif
	free(new_board);
	return NULL;
}

int main (int argc, char **argv)
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

	board = malloc(9*9*sizeof(*board));
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
		printf("Impossible :(\n");
	}
	free(board);
	return 0;
}
