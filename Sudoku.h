#ifndef SUDOKU_H
#define SUDOKU_H
#include <vector>
#include <string>

// Functions relating to the dancing links component of algorithmX
namespace dl
{
	struct Column;

	struct Cell
	{
		Cell* up;
		Cell* down;
		Cell* left;
		Cell* right;
		Column* col;
		int32_t row; //todo remove and calculate based on index
	};

	struct Column : public Cell
	{
		int32_t count;
		std::string name;

		Column() : count(0) {}
	};

	void cover(Column* cell);
	void uncover(Column* cell);
}
namespace
{
	class Sudoku
	{
		// Constraints here are 81 * 4,
		// first 81 refer to each 9x9 cell being occupied
		// second 81 refer to rowX having each 1-9
		// third 81 refer to colX having each 1-9
		// fourth 81 refer to each subgrid each 1-9
		static int32_t constexpr grid_size = 81;
		static int32_t constexpr constraints = grid_size * 4;

		// 81 cells in the sudoku, 9 choices for each
		static int32_t constexpr row_size = 729;

		// Each row always and only satifies four columns
		static int32_t constexpr cells_per_row = 4;

		// Keep a meta column header with extra spot for a root node
		static int32_t constexpr column_size = constraints + 1;

		int32_t row_count = 0;
		dl::Column columns[column_size];
		dl::Cell cells[row_size][cells_per_row];
		dl::Column* root;
		int32_t solution[grid_size];

	public:
		Sudoku();

		/**
			Function for inserting a row into the bottom of the dancing links matrix
		*/
		void insertRow(std::array<int32_t, cells_per_row> const& items);
		void loadGridAndSolve(std::vector<std::vector<int32_t>> const& grid);
	private:

		void printSolution();

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
		void search(int32_t k = 0);
	};
}
#endif // SUDOKU_H