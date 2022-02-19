/**
	Dancing links Sudoku solver - Richard Haar 2022

	Solves a sudoku by representing the problem as an exact cover problem
	and solves it using Donald Knuth's AlgorithmX with dancing links
	for effecient backtracking.

	See Donald Knuth's paper here: https://arxiv.org/pdf/cs/0011047.pdf

	Zendoku also has a good article explaining the dancing links algorithm:
	https://garethrees.org/2007/06/10/zendoku-generation/
*/
#include "Sudoku.h"

#include <cstdint>
#include <iostream>
#include <array>

int main()
{
	Sudoku sudoku;

	// 19/02/2022 NYTimes hard sudoku
	std::vector<std::vector<int32_t>> grid = {
		{0,7,0, 4,8,0, 1,3,0},
		{0,0,0, 0,0,0, 0,0,0},
		{0,0,0, 5,6,0, 0,8,0},

		{0,6,0, 0,0,8, 0,7,0},
		{0,4,1, 0,0,6, 0,0,0},
		{0,0,8, 0,0,0, 0,1,0},

		{0,9,0, 3,0,0, 2,0,8},
		{0,0,5, 0,0,2, 0,0,0},
		{4,0,0, 0,7,0, 5,0,0}
	};

	sudoku.loadGridAndSolve(grid);

	return 0;
}

namespace dl
{
	// Cover a column
	void cover(Column* cell)
	{
		// remove the column header
		cell->right->left = cell->left;
		cell->left->right = cell->right;

		// for each cell in the column
		for (Cell* i = cell->down; i != cell; i = i->down)
		{
			// and for each cell in the row, remove it
			for (Cell* j = i->right; j != i; j = j->right)
			{
				j->down->up = j->up;
				j->up->down = j->down;
				--j->col->count;
			}
		}
	}

	// Uncover a column
	void uncover(Column* cell)
	{
		// for each cell in the column, from bottom up
		for (Cell* i = cell->up; i != cell; i = i->up)
		{
			// and for each cell in the row, from left to right, add it back in
			for (Cell* j = i->left; j != i; j = j->left)
			{
				++j->col->count;
				j->down->up = j;
				j->up->down = j;
			}
		}

		// add the column header back in
		cell->right->left = cell;
		cell->left->right = cell;
	}
}

namespace
{
	Sudoku::Sudoku()
	{
		// construct columns for dancing links
		for (int32_t i = 0; i < column_size; ++i)
		{
			// columns begin as only item in the column
			columns[i].up = &columns[i];
			columns[i].down = &columns[i];

			// set right and left values
			columns[i].right = &columns[(i + 1) % column_size];
			columns[i].left = &columns[(column_size - 1 + i) % column_size];

			// set metadata
			columns[i].count = 0;
			columns[i].col = &columns[i];
			columns[i].name = "column " + std::to_string(i);
		}

		// Traditionally Donald knuths paper uses the root on the left, here
		// a right most column is used as a root, to conserve indexing from 0
		root = &columns[column_size - 1];

		// Construct all 729 rows, each column should have 9 items
		for (int32_t i = 0; i < row_size; ++i)
		{
			int32_t const col = (i / 9) % 9;
			int32_t const row = i / 81;
			int32_t const num = i % 9;

			int32_t const col_block_group = col / 3;
			int32_t const row_block_group = row / 3;

			int32_t const cell_constraint = i / 9;
			int32_t const row_constraint = (row * 9) + num;
			int32_t const column_constraint = i % 81;
			int32_t const block_constraint = row_block_group * 27 + col_block_group * 9 + num;

			insertRow({ cell_constraint, 81 + row_constraint, 162 + column_constraint, 243 + block_constraint });
		}
	}

	/**
		Function for inserting a row into the bottom of the dancing links matrix
	*/
	void Sudoku::insertRow(std::array<int32_t, cells_per_row> const& items)
	{
		for (int32_t i = 0; i < cells_per_row; ++i)
		{
			auto* const cell = &cells[row_count][i];

			cell->col = &columns[items[i]];
			cell->row = row_count;
			++cell->col->count;

			// insert vertically
			cell->up = cell->col->up;
			cell->down = cell->col;

			cell->col->up->down = cell;
			cell->col->up = cell;

			// insert horizontally
			cell->right = &cells[row_count][(i + 1) % cells_per_row];
			cell->left = &cells[row_count][(cells_per_row - 1 + i) % cells_per_row];
		}
		++row_count;
	}

	void Sudoku::printSolution()
	{
		// Decode matrix row to grid (row, col, value)
		int32_t grid_solution[9][9];

		for (int32_t k = 0; k < grid_size; ++k)
		{
			int32_t const col = (solution[k] / 9) % 9;
			int32_t const row = solution[k] / 81;
			int32_t const num = solution[k] % 9;
			grid_solution[row][col] = num;
		}

		std::cout << "-----" << std::endl;
		for (int32_t i = 0; i < 9; ++i)
		{
			for (int32_t j = 0; j < 9; ++j)
			{
				std::cout << (grid_solution[i][j] + 1) << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "-----" << std::endl;
	}

	/**
		Search algorithm as defined by AlgorithmX

		1. pick a candidate Column, the column with the least
			entries will limit branching.
		2. pick a row in that column, and for each cell in that
			row, cover it's column
		3. repeat step 1 until no column remains and thus a solution
			is found or until there are columns with 0 entries
			and then the column is uncovered again and the previous
			row decision is undone and a new row is tried

	*/
	void Sudoku::search(int32_t k)
	{
		if (root->right == root)
		{
			printSolution();
			return;
		}

		dl::Cell* c = root->right;
		// By picking the column with the least amount of entries we can limit
		// the amount of branching we do
		for (dl::Cell* candidate = root->right; candidate != root; candidate = candidate->right)
		{
			if (candidate->col->count < c->col->count)
			{
				c = candidate;
			}
		}

		cover(c->col);

		for (dl::Cell* r = c->down; r != c; r = r->down)
		{
			solution[k] = r->row;

			for (dl::Cell* j = r->right; j != r; j = j->right)
			{
				dl::cover(j->col);
			}

			search(k + 1);

			for (dl::Cell* j = r->left; j != r; j = j->left)
			{
				dl::uncover(j->col);
			}

		}
		dl::uncover(c->col);
	}

	/**
		Simulate the state of the dancing links matrix asif the algorithm had
		picked the rows corresponding to the current layout of the sudoku
	*/
	void Sudoku::loadGridAndSolve(std::vector<std::vector<int32_t>> const& grid)
	{
		int32_t z = 0;

		for (int32_t i = 0; i < 9; ++i)
		{
			for (int32_t j = 0; j < 9; ++j)
			{
				if (grid[i][j] != 0)
				{
					// find which row this corresponds to
					int32_t const grid_row = (81 * i) + (9 * j) + grid[i][j] - 1;

					for (int32_t k = 0; k < cells_per_row; ++k)
					{
						dl::cover(cells[grid_row][k].col);
					}
					solution[z] = grid_row;
					++z;
				}
			}
		}

		search(z);
	}
}