#include "game.h"
#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <string.h>

#include "grid.h"
#include "constants.h"

static void DrawGameplay(void);
static void DrawStartScreen(void);
static void DrawLoseScreen(void);
static void DrawWinScreen(void);

static void AddPiece(void);
static void FindHighestPiece(void);

typedef enum GameState
{

	START = 0,
	GAMEPLAY,
	LOSS,
	WIN

} GameState;

float _stateTimer = 0;
GameState _state = START;
GameState _stateDest = START;
float _delayedStateChange = 0;
GameState _delayedState = START;
float _gameStartTime = 0;
float _gameEndTime = 0;

bool InputBlocked(void)
{
	return _stateTimer > 0 || _delayedStateChange > 0;
}

void StateChange(GameState destination)
{
	_stateDest = destination;
	_stateTimer = STATE_TRANSITION_TIME;

	if (destination == LOSS)
	{
		FindHighestPiece();
	}
}

void SetDelayedStateChange(GameState destination)
{
	_delayedStateChange = STATE_DELAY_TIME;
	_delayedState = destination;

	if (destination == WIN)
	{
		_gameEndTime = GetTime();
	}
}

typedef enum PieceValue
{

	NONE = -1,
	TWO = 0,
	FOUR,
	EIGHT,
	SIXTEEN,
	THIRTY_TWO,
	SIXTY_FOUR,
	ONE_TWENTY_EIGHT,
	TWO_FIFTY_SIX,
	FIVE_TWELVE,
	TEN_TWENTY_FOUR,
	TWENTY_FORTY_EIGHT,

} PieceValue;

typedef struct Piece
{
	PieceValue value;

	Vector2 startPosition;
	Vector2 endPosition;

	float animationTime;
	bool isSpawning;
} Piece;

void PieceReset(Piece* piece)
{
	piece->value = NONE;
	piece->animationTime = 0;
	piece->startPosition = (Vector2){0};
	piece->endPosition = (Vector2){0};
	piece->isSpawning = false;
}

void PieceSetFromSpawn(Piece* piece, PieceValue value)
{
	piece->value = value;
	piece->animationTime = PIECE_SPAWN_TIME;
	piece->isSpawning = true;
}

Grid _grid;
Piece _pieces[GRID_COLS][GRID_ROWS] = { 0 };
PieceValue _highestPiece = TWO;

void FindHighestPiece(void)
{
	_highestPiece = TWO;
	for (int x = 0; x < GRID_COLS; x++)
	{
		for (int y = 0; y < GRID_ROWS; y++)
		{
			if (_pieces[x][y].value > _highestPiece)
			{
				_highestPiece = _pieces[x][y].value;
			}
		}
	}
}

void BeginGame(void)
{
	StateChange(GAMEPLAY);

	_grid = (Grid){ 0 };
	_grid.backgroundColor = GRAY;
	_grid.outlineColor = BLACK;
	_grid.gridlineColor = DARKGRAY;
	_grid.outlineThickness = 8;
	_grid.gridlineThickness = 4;
	_grid.cols = GRID_COLS;
	_grid.rows = GRID_ROWS;
	_grid.destination = (Rectangle){ 100, 100, 600, 600 };

	_gameStartTime = GetTime();

	for (int x = 0; x < GRID_COLS; x++)
	{
		for (int y = 0; y < GRID_ROWS; y++)
		{
			_pieces[x][y] = (Piece){0};
			_pieces[x][y].value = NONE;
		}
	}

	AddPiece();
	AddPiece();
}

void DrawGame(void)
{
	if (_stateTimer > 0)
	{
		_stateTimer -= GetFrameTime();
		if (_stateTimer <= 0 && _state != _stateDest)
		{
			_state = _stateDest;
			_stateTimer = STATE_TRANSITION_TIME;
		}
	}

	if (_delayedStateChange > 0)
	{
		_delayedStateChange -= GetFrameTime();
		if (_delayedStateChange <= 0)
		{
			StateChange(_delayedState);
		}
	}

	switch (_state)
	{
	case START:
		DrawStartScreen();
		break;
	case GAMEPLAY:
		DrawGameplay();
		break;
	case LOSS:
		DrawLoseScreen();
		break;
	case WIN:
		DrawWinScreen();
		break;
	}
}

Color ColorFromPieceValue(PieceValue value)
{
	if (value == NONE)
	{
		return BLANK;
	}

	float t = (float)value / (float)TWENTY_FORTY_EIGHT;
	return ColorFromHSV(Lerp(60.0f, 0.0f, t), Lerp(0.2f, 1.0f, t), 1.0f);
}

const char* TextFromPieceValue(PieceValue value)
{
	switch (value)
	{
		case NONE:
			return "ERROR";
		case TWO:
			return "2";
		case FOUR:
			return "4";
		case EIGHT:
			return "8";
		case SIXTEEN:
			return "16";
		case THIRTY_TWO:
			return "32";
		case SIXTY_FOUR:
			return "64";
		case ONE_TWENTY_EIGHT:
			return "128";
		case TWO_FIFTY_SIX:
			return "256";
		case FIVE_TWELVE:
			return "512";
		case TEN_TWENTY_FOUR:
			return "1024";
		case TWENTY_FORTY_EIGHT:
			return "2048";
	}
	
	return "UNDEFINED";
}

void PiecesDraw(void)
{
	float ft = GetFrameTime();
	Vector2 gridCellSize = GridCellSize(_grid);

	for (int x = 0; x < GRID_COLS; x++)
	{
		for (int y = 0; y < GRID_ROWS; y++)
		{
			if (_pieces[x][y].value == NONE)
			{
				continue;
			}

			Piece* p = &(_pieces[x][y]);
			p->animationTime -= ft;

			bool animateColor = p->isSpawning && p->animationTime > 0;
			bool animatePosition = !p->isSpawning && p->animationTime > 0;

			Color color = ColorFromPieceValue(p->value);
			if (animateColor)
			{
				float fadeAmount = Lerp(0.0f, 1.0f, (PIECE_SPAWN_TIME - p->animationTime) / PIECE_SPAWN_TIME);
				color = Fade(color, fadeAmount);
			}

			Vector2 position = !animatePosition ? GridCellPosition2(_grid, x, y)
				: Vector2Lerp(p->startPosition, p->endPosition, (PIECE_MOVE_TIME - p->animationTime) / PIECE_MOVE_TIME);

			DrawRectangleV(position, gridCellSize, color);
			DrawText(TextFromPieceValue(p->value), position.x + 4, position.y + 4, 24, BLACK);
		}
	}
}

void CheckForLoseState(void)
{
	for (int x = 0; x < GRID_COLS; x++)
	{
		for (int y = 0; y < GRID_ROWS; y++)
		{
			if (_pieces[x][y].value == NONE)
			{
				return;
			}

			if (x != GRID_COLS - 1)
			{
				if (_pieces[x][y].value == _pieces[x + 1][y].value)
				{
					return;
				}
			}

			if (y != GRID_ROWS - 1)
			{
				if (_pieces[x][y].value == _pieces[x][y + 1].value)
				{
					return;
				}
			}
		}
	}

	SetDelayedStateChange(LOSS);
}

void AddPiece(void)
{
	int size = GRID_COLS * GRID_ROWS;
	int start = GetRandomValue(0, size);

	int checks = 0;
	for (; checks < 16; checks++)
	{
		Piece* piece = &(_pieces[start % GRID_COLS][start / GRID_ROWS]);
		if (piece->value == NONE)
		{
			PieceSetFromSpawn(piece, GetRandomValue(0, 4) == 0 ? FOUR : TWO);
			CheckForLoseState();
			return;
		}

		start = (start + 1) % size;
	}
	
	TraceLog(LOG_ERROR, "Could not find an index to spawn a piece in AddPiece!");
}

bool PiecesMove(Vector2 input)
{
	bool pieceMoved = false;

	if (input.x > 0)
	{
		for (int x = GRID_COLS - 1; x >= 0; x--)
		{
			for (int y = 0; y < GRID_ROWS; y++)
			{
				if (_pieces[x][y].value == NONE)
				{
					continue;
				}
				
				int newX = x;

				while (newX < GRID_COLS - 1)
				{
					if (_pieces[newX + 1][y].value != NONE)
					{
						if (_pieces[x][y].value == _pieces[newX + 1][y].value)
						{
							PieceReset(&(_pieces[x][y]));

							_pieces[newX + 1][y].value++;
							_pieces[newX + 1][y].startPosition = GridCellPosition2(_grid, x, y);
							_pieces[newX + 1][y].endPosition = GridCellPosition2(_grid, newX + 1, y);
							_pieces[newX + 1][y].animationTime = PIECE_MOVE_TIME;

							if (_pieces[newX + 1][y].value == TWENTY_FORTY_EIGHT)
							{
								SetDelayedStateChange(WIN);
							}

							continue;
						}

						break;
					}

					newX++;
				}

				if (newX != x || _pieces[x][y].value == NONE)
				{
					pieceMoved = true;
				}

				PieceValue value = _pieces[x][y].value;
				PieceReset(&(_pieces[x][y]));
				_pieces[newX][y].value = value;

				_pieces[newX][y].startPosition = GridCellPosition2(_grid, x, y);
				_pieces[newX][y].endPosition = GridCellPosition2(_grid, newX, y);
				_pieces[newX][y].animationTime = PIECE_MOVE_TIME;
			}
		}
	}
	else if (input.x < 0)
	{
		for (int x = 0; x < GRID_COLS; x++)
		{
			for (int y = 0; y < GRID_ROWS; y++)
			{
				if (_pieces[x][y].value == NONE)
				{
					continue;
				}

				int newX = x;

				while (newX > 0)
				{
					if (_pieces[newX - 1][y].value != NONE)
					{
						if (_pieces[x][y].value == _pieces[newX - 1][y].value)
						{
							PieceReset(&(_pieces[x][y]));

							_pieces[newX - 1][y].value++;
							_pieces[newX - 1][y].startPosition = GridCellPosition2(_grid, x, y);
							_pieces[newX - 1][y].endPosition = GridCellPosition2(_grid, newX - 1, y);
							_pieces[newX - 1][y].animationTime = PIECE_MOVE_TIME;
							

							if (_pieces[newX - 1][y].value == TWENTY_FORTY_EIGHT)
							{
								SetDelayedStateChange(WIN);
							}

							continue;
						}

						break;
					}

					newX--;
				}

				if (newX != x || _pieces[x][y].value == NONE)
				{
					pieceMoved = true;
				}
				
				PieceValue value = _pieces[x][y].value;
				PieceReset(&(_pieces[x][y]));
				_pieces[newX][y].value = value;

				_pieces[newX][y].startPosition = GridCellPosition2(_grid, x, y);
				_pieces[newX][y].endPosition = GridCellPosition2(_grid, newX, y);
				_pieces[newX][y].animationTime = PIECE_MOVE_TIME;
			}
		}
	}
	
	if (input.y < 0)
	{
		for (int x = 0; x < GRID_COLS; x++)
		{
			for (int y = 0; y < GRID_ROWS; y++)
			{
				if (_pieces[x][y].value == NONE)
				{
					continue;
				}

				int newY = y;

				while (newY > 0)
				{
					if (_pieces[x][newY - 1].value != NONE)
					{
						if (_pieces[x][y].value == _pieces[x][newY - 1].value)
						{
							PieceReset(&(_pieces[x][y]));

							_pieces[x][newY - 1].value++;
							_pieces[x][newY - 1].startPosition = GridCellPosition2(_grid, x, y);
							_pieces[x][newY - 1].endPosition = GridCellPosition2(_grid, x, newY - 1);
							_pieces[x][newY - 1].animationTime = PIECE_MOVE_TIME;

							if (_pieces[x][newY - 1].value == TWENTY_FORTY_EIGHT)
							{
								SetDelayedStateChange(WIN);
							}

							continue;
						}

						break;
					}

					newY--;
				}

				if (newY != y || _pieces[x][y].value == NONE)
				{
					pieceMoved = true;
				}
				
				PieceValue value = _pieces[x][y].value;
				PieceReset(&(_pieces[x][y]));
				_pieces[x][newY].value = value;

				_pieces[x][newY].startPosition = GridCellPosition2(_grid, x, y);
				_pieces[x][newY].endPosition = GridCellPosition2(_grid, x, newY);
				_pieces[x][newY].animationTime = PIECE_MOVE_TIME;
			}
		}
	}
	else if (input.y > 0)
	{
		for (int x = 0; x < GRID_COLS; x++)
		{
			for (int y = GRID_ROWS - 1; y >= 0; y--)
			{
				if (_pieces[x][y].value == NONE)
				{
					continue;
				}

				int newY = y;

				while (newY < GRID_COLS - 1)
				{
					if (_pieces[x][newY + 1].value != NONE)
					{
						if (_pieces[x][y].value == _pieces[x][newY + 1].value)
						{
							PieceReset(&(_pieces[x][y]));

							_pieces[x][newY + 1].value++;
							_pieces[x][newY + 1].startPosition = GridCellPosition2(_grid, x, y);
							_pieces[x][newY + 1].endPosition = GridCellPosition2(_grid, x, newY + 1);
							_pieces[x][newY + 1].animationTime = PIECE_MOVE_TIME;

							if (_pieces[x][newY + 1].value == TWENTY_FORTY_EIGHT)
							{
								SetDelayedStateChange(WIN);
							}

							continue;
						}

						break;
					}

					newY++;
				}

				if (newY != y || _pieces[x][y].value == NONE)
				{
					pieceMoved = true;
				}
				
				PieceValue value = _pieces[x][y].value;
				PieceReset(&(_pieces[x][y]));
				_pieces[x][newY].value = value;

				_pieces[x][newY].startPosition = GridCellPosition2(_grid, x, y);
				_pieces[x][newY].endPosition = GridCellPosition2(_grid, x, newY);
				_pieces[x][newY].animationTime = PIECE_MOVE_TIME;
			}
		}
	}

	return pieceMoved;
}

const char* GetPlayTime(void)
{
	float timePlayed = _gameEndTime - _gameStartTime;
	int minutes = (int)timePlayed / 60;
	int seconds = (int)timePlayed % 60;

	return TextFormat("%dm %ds", minutes, seconds);
}

void DrawGameplay(void)
{
	if (!InputBlocked())
	{
		Vector2 input = { IsKeyPressed(KEY_RIGHT) - IsKeyPressed(KEY_LEFT), IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP) };
		if (PiecesMove(input))
		{
			AddPiece();
		}
	}

	BeginDrawing();
	{
		ClearBackground(RAYWHITE);

		GridDraw(_grid);

		PiecesDraw();

		if (_stateTimer > 0)
		{
			float t = _state != _stateDest ? (STATE_TRANSITION_TIME - _stateTimer) / STATE_TRANSITION_TIME : _stateTimer / STATE_TRANSITION_TIME;
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, t));
		}
	}
	EndDrawing();
}

void DrawStartScreen(void)
{
	const char* text = "Press SPACE to Start!";
	const int fontSize = 40;
	const int x = (SCREEN_WIDTH - MeasureText(text, fontSize)) * 0.5f;

	if (!InputBlocked() && IsKeyPressed(KEY_SPACE))
	{
		BeginGame();
	}

	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawText(text, x, 100, fontSize, BLACK);

		if (_stateTimer > 0)
		{
			float t = _state != _stateDest ? (STATE_TRANSITION_TIME - _stateTimer) / STATE_TRANSITION_TIME : _stateTimer / STATE_TRANSITION_TIME;
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, t));
		}
	}
	EndDrawing();
}

void DrawLoseScreen(void)
{
	const char* text1 = "Game Over!";
	const int fontSize1 = 60;
	const char* text2 = "Press 'R' to try again";
	const int fontSize2 = 45;
	const char* text3 = TextFormat("Highest Piece Value: %s", TextFromPieceValue(_highestPiece));
	const int fontSize3 = 60;

	const int x1 = (SCREEN_WIDTH - MeasureText(text1, fontSize1)) * 0.5f;
	const int x2 = (SCREEN_WIDTH - MeasureText(text2, fontSize2)) * 0.5f;
	const int x3 = (SCREEN_WIDTH - MeasureText(text3, fontSize3)) * 0.5f;

	if (!InputBlocked() && IsKeyPressed(KEY_R))
	{
		BeginGame();
	}

	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawText(text1, x1, 330, fontSize1, BLACK);
		DrawText(text2, x2, 400, fontSize2, GRAY);
		DrawText(text3, x3, 500, fontSize3, BLACK);
		
		if (_stateTimer > 0)
		{
			float t = _state != _stateDest ? (STATE_TRANSITION_TIME - _stateTimer) / STATE_TRANSITION_TIME : _stateTimer / STATE_TRANSITION_TIME;
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, t));
		}
	}
	EndDrawing();
}

void DrawWinScreen(void)
{
	const char* text1 = "You Win!";
	const int fontSize1 = 60;
	const char* text2 = "Press 'R' to play again";
	const int fontSize2 = 45;
	const char* text3 = TextFormat("Time Played: %s", GetPlayTime());
	const int fontSize3 = 60;

	const int x1 = (SCREEN_WIDTH - MeasureText(text1, fontSize1)) * 0.5f;
	const int x2 = (SCREEN_WIDTH - MeasureText(text2, fontSize2)) * 0.5f;
	const int x3 = (SCREEN_WIDTH - MeasureText(text3, fontSize3)) * 0.5f;

	if (!InputBlocked() && IsKeyPressed(KEY_R))
	{
		BeginGame();
	}

	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawText(text1, x1, 330, fontSize1, BLACK);
		DrawText(text2, x2, 400, fontSize2, GRAY);
		DrawText(text3, x3, 500, fontSize3, BLACK);
		
		if (_stateTimer > 0)
		{
			float t = _state != _stateDest ? (STATE_TRANSITION_TIME - _stateTimer) / STATE_TRANSITION_TIME : _stateTimer / STATE_TRANSITION_TIME;
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, t));
		}
	}
	EndDrawing();
}