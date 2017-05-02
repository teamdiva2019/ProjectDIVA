// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define HEIGHT 256
#define WIDTH 256

struct Shape {
public:
	int xCoord;
	int yCoord;
	int height;
	int width;
	char **figure;
};

struct Glider : public Shape {
	static const int GLIDER_SIZE = 3;
	Glider(int x, int y);
	~Glider();
};

struct Acorn : public Shape {
	Acorn(int x, int y);
	~Acorn();
};

struct Blinker : public Shape {
	static const int BLINKER_HEIGHT = 3;
	static const int BLINKER_WIDTH = 1;
	Blinker(int x, int y);
	~Blinker();
};

class DYNAMICTEXTURESAMPLE_API GameOfLife {
public:
	GameOfLife();
	GameOfLife(Shape sh);
	void update();
	char getState(char state, int xCoord, int yCoord, bool toggle);
	bool getCell(unsigned int x, unsigned int y);

	void iterate(unsigned int iterations);
private:
	char world[HEIGHT][WIDTH];
	char otherWorld[HEIGHT][WIDTH];
	bool toggle;
	Shape shape;
};