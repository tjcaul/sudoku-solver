#include <string.h>
#include "check.h"
#include "io.h"
#include "debug.h"

int check_row(int *board, int row)
{
	//debug("check_row(%p, %d)\n", board, row);
	int full[9];
	memset(full, 0, 9 * sizeof(*full));
	for (int i = 0; i < 9; ++i) {
		int val = board[row * 9 + i];
		if (val == 0)
			continue;
		++full[val - 1];
		if (full[val - 1] > 1)
			return 0;
	}
	return 1;
}

int check_col(int *board, int col)
{
	int full[9];
	memset(full, 0, 9 * sizeof(*full));
	for (int i = 0; i < 9; ++i) {
		int val = board[i * 9 + col];
		if (val == 0)
			continue;
		++full[val - 1];
		if (full[val - 1] > 1)
			return 0;
	}
	return 1;
}

int check_box(int *board, int row, int col)
{
	row = (row / 3) * 3;
	col = (col / 3) * 3;
	int full[9];
	memset(full, 0, 9 * sizeof(*full));
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int val = board[row * 9 + col + i * 9 + j];
			if (val == 0)
				continue;
			++full[val - 1];
			if (full[val - 1] > 1)
				return 0;
		}
	}
	return 1;
}

int check_board(int *board, int cell)
{
	int row = cell / 9;
	int col = cell % 9;
	int valid = check_row(board, row) && check_col(board, col)
		&& check_box(board, row, col);
	//debug("Checking row %d, col %d: %s\n", row, col, valid?"valid":"invalid");
	return valid;
}
