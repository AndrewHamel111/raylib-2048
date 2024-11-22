#ifndef GRID_H_
#define GRID_H_

#include "raylib.h"

typedef struct Grid
{

	Rectangle destination;
	int rows;
	int cols;

	int outlineThickness;
	Color outlineColor;

	int gridlineThickness;
	Color gridlineColor;

	Color backgroundColor;

} Grid;

void GridDraw(Grid grid);
Vector2 GridCellSize(Grid grid);
Vector2 GridCellPosition(Grid grid, int index);
Vector2 GridCellPosition2(Grid grid, int col, int row);

#endif