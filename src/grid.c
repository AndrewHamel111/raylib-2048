#include "grid.h"

void GridDraw(Grid grid)
{
	int oT = grid.outlineThickness;

	DrawRectangleRec(grid.destination, grid.backgroundColor);
	DrawRectangleLinesEx(grid.destination, oT, grid.outlineColor);

	int cellWidth = (grid.destination.width - (oT * 2)) / grid.cols;
	int cellHeight = (grid.destination.height - (oT * 2)) / grid.rows;

	Rectangle dest = (Rectangle){ grid.destination.x + oT, grid.destination.y + oT, cellWidth, cellHeight };
	
	for (int y = 0; y < grid.rows; y++)
	{
		for (int x = 0; x < grid.cols; x++)
		{
			Rectangle d = dest;
			d.x += x * cellWidth;
			d.y += y * cellHeight;
			DrawRectangleLinesEx(d, grid.gridlineThickness, grid.gridlineColor);
		}
	}
}

Vector2 GridCellSize(Grid grid)
{
	float x = (grid.destination.width - (grid.outlineThickness * 2)) / grid.cols;
	return (Vector2){ x, x };
}

Vector2 GridCellPosition(Grid grid, int index)
{
	return GridCellPosition2(grid, index % grid.cols, index / grid.rows);
}

Vector2 GridCellPosition2(Grid grid, int col, int row)
{
	float cellSz = GridCellSize(grid).x;
	return (Vector2)
	{
		grid.destination.x + grid.outlineThickness + (col * cellSz),
		grid.destination.y + grid.outlineThickness + (row * cellSz)
	};
}
