#include <string.h>
#include "io.h"
#include "debug.h"

int load_board(int *board, FILE *file)
{
	for (int i = 0; i < 9 * 9;) {
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

void print_board(int *board)
{
	puts(".-----.-----.-----.");
	for (int i = 0; i < 9; ++i) {
		putchar('|');
		for (int j = 0; j < 9; ++j) {
			char c = board[i * 9 + j] + '0';
			if (c == '0')
				c = ' ';
			putchar(c);
			if (j % 3 == 2)
				putchar('|');
			else
				putchar(' ');
		}
		putchar('\n');
		if (i == 8)
			puts("'-----'-----'-----'");
		else if (i % 3 == 2)
			puts("|-----+-----+-----|");
	}
}
