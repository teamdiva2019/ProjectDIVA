// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicTextureSample.h"
#include "GameOfLife.h"

GameOfLife::GameOfLife() :
toggle(true)
{
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			world[i][j] = '.';
		}
	}

	Acorn *shape = new Acorn(100, 100);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {

			char chara[] = { shape->figure[i][j], 0 };

			//OutputDebugStringW(ANSI_TO_TCHAR(chara));
		}
	}

	for (int i = shape->yCoord; i - shape->yCoord < shape->height; i++) {
		for (int j = shape->xCoord; j - shape->xCoord < shape->width; j++) {
			if (i < HEIGHT && j < WIDTH) {
				world[i][j] = shape->figure[i - shape->yCoord][j - shape->xCoord];
			}
		}
	}
}

GameOfLife::GameOfLife(Shape sh) :
shape(sh),
toggle(true)
{
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			world[i][j] = '.';
		}
	}
	for (int i = shape.yCoord; i - shape.yCoord < shape.height; i++) {
		for (int j = shape.xCoord; j - shape.xCoord < shape.width; j++) {
			if (i < HEIGHT && j < WIDTH) {
				world[i][j] =
					shape.figure[i - shape.yCoord][j - shape.xCoord];
			}
		}
	}
}

void GameOfLife::update() {
	if (toggle) {
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				otherWorld[i][j] =
					GameOfLife::getState(world[i][j], i, j, toggle);
			}
		}
		toggle = !toggle;
	}
	else {
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				world[i][j] =
					GameOfLife::getState(otherWorld[i][j], i, j, toggle);
			}
		}
		toggle = !toggle;
	}
}

char GameOfLife::getState(char state, int yCoord, int xCoord, bool toggle) {
	int neighbors = 0;
	if (toggle) {
		for (int i = yCoord - 1; i <= yCoord + 1; i++) {
			for (int j = xCoord - 1; j <= xCoord + 1; j++) {
				if (i == yCoord && j == xCoord) {
					continue;
				}
				if (i > -1 && i < HEIGHT && j > -1 && j < WIDTH) {
					if (world[i][j] == 'X') {
						neighbors++;
					}
				}
			}
		}
	}
	else {
		for (int i = yCoord - 1; i <= yCoord + 1; i++) {
			for (int j = xCoord - 1; j <= xCoord + 1; j++) {
				if (i == yCoord && j == xCoord) {
					continue;
				}
				if (i > -1 && i < HEIGHT && j > -1 && j < WIDTH) {
					if (otherWorld[i][j] == 'X') {
						neighbors++;
					}
				}
			}
		}
	}
	if (state == 'X') {
		return (neighbors > 1 && neighbors < 4) ? 'X' : '.';
	}
	else {
		return (neighbors == 3) ? 'X' : '.';
	}
}

bool GameOfLife::getCell(unsigned int x, unsigned int y)
{
	if (toggle)
	{
		return world[x][y] == 'X';
	}
	else
	{
		return otherWorld[x][y] == 'X';
	}
}

void GameOfLife::iterate(unsigned int iterations) {
	for (unsigned int i = 0; i < iterations; i++) {
		update();
	}
}

Glider::Glider(int x, int y) {
	xCoord = x;
	yCoord = y;
	height = GLIDER_SIZE;
	width = GLIDER_SIZE;
	figure = new char*[GLIDER_SIZE];
	for (int i = 0; i < GLIDER_SIZE; i++) {
		figure[i] = new char[GLIDER_SIZE];
	}
	for (int i = 0; i < GLIDER_SIZE; i++) {
		for (int j = 0; j < GLIDER_SIZE; j++) {
			figure[i][j] = '.';
		}
	}
	figure[0][1] = 'X';
	figure[1][2] = 'X';
	figure[2][0] = 'X';
	figure[2][1] = 'X';
	figure[2][2] = 'X';
}

Glider::~Glider() {
	for (int i = 0; i < GLIDER_SIZE; i++) {
		delete[] figure[i];
	}
	delete[] figure;
}

Blinker::Blinker(int x, int y) {
	xCoord = x;
	yCoord = y;
	height = BLINKER_HEIGHT;
	width = BLINKER_WIDTH;
	figure = new char*[BLINKER_HEIGHT];

	for (int i = 0; i < BLINKER_HEIGHT; i++) {
		figure[i] = new char[BLINKER_WIDTH];
	}
	for (int i = 0; i < BLINKER_HEIGHT; i++) {
		for (int j = 0; j < BLINKER_WIDTH; j++) {
			figure[i][j] = 'X';
		}
	}
}

Blinker::~Blinker() {
	for (int i = 0; i < BLINKER_HEIGHT; i++) {
		delete[] figure[i];
	}
	delete[] figure;
}


Acorn::Acorn(int x, int y)
{
	xCoord = x;
	yCoord = y;

	height = 3;
	width = 7;

	figure = new char*[height];

	for (int i = 0; i < height; i++) {
		figure[i] = new char[width];
	}

	figure[0][1] = 'X';
	figure[1][3] = 'X';
	figure[2][0] = 'X';
	figure[2][1] = 'X';
	figure[2][4] = 'X';
	figure[2][5] = 'X';
	figure[2][6] = 'X';
}

Acorn::~Acorn()
{

}
