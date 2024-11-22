#include "raylib.h"

#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "constants.h"

int main(void)
{
	srand(time(NULL));
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2048");

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
		DrawGame();
	}

	CloseWindow();

	return 0;
}