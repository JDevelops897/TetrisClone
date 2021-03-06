﻿//TETRIS CLONE C++

//by 

//Jordan Brown

//#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <Windows.h>
#include <algorithm>
#include <ctime>

using namespace std;

//access the console handle for console functions
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

//bool values for if the keys are being held
bool keyPressedUp = false;
bool keyPressedDown = false;
bool keyPressedLeft = false;
bool keyPressedRight = false;
bool keyPressedRotateRight = false;
bool keyPressedRotateLeft = false;

//attempted frames per second
CONST int framerate = 60;

//set the 10x20 field for the tetris game
int field[10][20];

//define the tetromino types
enum Type {
	n = 0,
	l = 1,
	j = 2,
	t = 3,
	z = 4,
	s = 5,
	i = 6,
	o = 7
};

//colors
enum Colors {
	c1 = 1,
	c2 = 9,
	c3 = 11
};
/*
The different color codes are

0   BLACK
1   BLUE
2   GREEN
3   CYAN
4   RED
5   MAGENTA
6   BROWN
7   LIGHTGRAY
8   DARKGRAY
9   LIGHTBLUE
10  LIGHTGREEN
11  LIGHTCYAN
12  LIGHTRED
13  LIGHTMAGENTA
14  YELLOW
15  WHITE
*/

//define the tetromino orientations
//each one is an array of four coord elements
//each coord represents the relative position of the four blocks

CONST COORD T1[4] = { {-1, 0}, {0,  0},  {1, 0}, {0, -1} }; // T up
CONST COORD T2[4] = { {0, -1}, {0,  0},  {1, 0}, {0, 1} };  // T right
CONST COORD T3[4] = { {-1, 0}, {0,  0},  {1, 0}, {0, 1} };  // T down (spawn)
CONST COORD T4[4] = { {0, -1}, {-1, 0}, {0, 0}, {0, 1} };   // T left

CONST COORD J1[4] = { { -1, -1 }, { -1,  0 }, { 0,  0 }, { 1,  0 } }; // J up
CONST COORD J2[4] = { {  0, -1 }, { 1, -1 }, { 0,  0 }, { 0,  1 } }; // J right
CONST COORD J3[4] = { { -1, 0 }, { 0,  0 }, { 1,  0 }, { 1,  1 } }; // J down(spawn)
CONST COORD J4[4] = { {  0, -1 }, { 0,  0 }, { -1,  1 }, { 0,  1 } }; // J left

CONST COORD Z1[4] = { { -1, 0 }, { 0,  0 }, { 0,  1 }, { 1,  1 }}; // Z horizontal(spawn)
CONST COORD Z2[4] = { {  1, -1 }, { 0,  0 }, { 1,  0 }, { 0,  1 }}; // Z vertical

CONST COORD O1[4] = { { -1, 0 }, { 0,  0 }, { -1,  1 }, { 0,  1 } }; // O(spawn)

CONST COORD S1[4] = { {  0, 0 }, { 1,  0 }, { -1,  1 }, { 0,  1 }}; // S horizontal(spawn)
CONST COORD S2[4] = { {  0, -1 }, { 0,  0 }, { 1,  0 }, { 1,  1 }}; // S vertical

CONST COORD L1[4] = { {  1, -1 }, { -1,  0 }, { 0,  0 }, { 1,  0 } }; // L up
CONST COORD L2[4] = { {  0, -1 }, { 0,  0 }, { 0,  1 }, { 1,  1 } }; // L right
CONST COORD L3[4] = { { -1, 0 }, { 0,  0 }, { 1,  0 }, { -1,  1 } }; // L down(spawn)
CONST COORD L4[4] = { { -1, -1 }, { 0, -1 }, { 0,  0 }, { 0,  1 }}; // L left

CONST COORD I1[4] = { { -2, 0 }, { -1,  0 }, { 0,  0 }, { 1,  0 } }; // I horizontal(spawn)
CONST COORD I2[4] = { {  0, -2 }, { 0, -1 }, { 0,  0 }, { 0,  1 } }; // I vertical

CONST COORD n1[4] = { { 0, 0 },{ 0, 0 },{ 0, 0 },{ 0, 0 } }; //nil, 0 block

//draw text at a postion on the screen with a certain color
void DrawTextAtPosition(string text, int color, COORD position) {
	SetConsoleCursorPosition(hConsole, { position.X , position.Y });
	SetConsoleTextAttribute(hConsole, color);
	cout << text;
	SetConsoleCursorPosition(hConsole, { 0 , 0 });
}

//checks if a position is within the bounds of the field
bool PositionOutOfBounds(int x, int y) {
	if (x < 0 || y < 0 || x > 9 || y > 19) return true;
	return false;
}

//blocks are single squares on the field
class Block
{
public:
	//position coord
	COORD pos;

	int color;

	Block();
	Block(short xx, short yy, int c)
	{
		pos = { xx, yy };
		color = c;
	}

	//move to position (not relative to current)
	void MoveTo(int x, int y) {
		//if the position it is moving to is not it's current position, draw a blank space in the old spot
		if (x != pos.X && y != pos.Y)
			DrawTextAtPosition(" ", 0, { pos.X + 1, pos.Y });
		pos.X = x;
		pos.Y = y;
		//move to new position and draw itself there
		DrawTextAtPosition(" ", color*16+color, { pos.X + 1, pos.Y });
	}
};

//tetrominos are made up of four blocks in various orientations
class Tetromino {
public:
	//position of the center of the mino
	COORD pos;

	//rotation is between 0 and 3
	int rotation;
	//the array that contains the position of each block
	COORD orientation[4];

	int type;
	int color;

	//pointers to the four blocks this mino consists of
	Block* b1;
	Block* b2;
	Block* b3;
	Block* b4;

	//constructor
	Tetromino();
	Tetromino(short xx, short yy, int t) {
		pos = { xx, yy };
		type = t;
		switch (type) {
			//set the color depending on the type
			case Type::t:
			case Type::o:
			case Type::i:
				color = Colors::c3;
				break;
			case Type::j:
			case Type::s:
				color = Colors::c1;
				break;
			case Type::l:
			case Type::z:
				color = Colors::c2;
				break;
		}
		//ALL DEFAULT ROTATIONS ARE 2
		rotation = 2;
		//create the four blocks that make up this mino
		b1 = new Block(pos.X, pos.Y, color);
		b2 = new Block(pos.X, pos.Y, color);
		b3 = new Block(pos.X, pos.Y, color);
		b4 = new Block(pos.X, pos.Y, color);
		//set the orientation of the blocks
		SetOrientation();
		//update the mino
		Update();

		//check if it's currently colliding with existing blocks
		if (Collision())	{
			//reset field
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 20; j++) {
					field[i][j] = 0;
				}
			}
		}
	}

	~Tetromino() { };

	//checks if any of the four blocks are colliding with existing blocks
	bool Collision() {
		return (field[b1->pos.X][b1->pos.Y] != 0 ||
			field[b2->pos.X][b2->pos.Y] != 0 ||
			field[b3->pos.X][b3->pos.Y] != 0 ||
			field[b4->pos.X][b4->pos.Y] != 0);
	}

	//sets the blocks into the correct orientation
	void SetOrientation() {
		switch (type) {
		//set orientation depending on type of rotation
		case Type::t:
			switch (rotation) {
			case 0:
				for (int i = 0; i < 4; i++) {
					orientation[i] = T1[i];
				}
				break;
			case 1:
				for (int i = 0; i < 4; i++) {
					orientation[i] = T2[i];
				}
				break;
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = T3[i];
				}
				break;
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = T4[i];
				}
				break;
			}
			break;
		case Type::z:
			switch (rotation) {
			case 0:
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = Z1[i];
				}
				break;
			case 1:
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = Z2[i];
				}
				break;
			}
			break;
		case Type::s:
			switch (rotation) {
			case 0:
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = S1[i];
				}
				break;
			case 1:
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = S2[i];
				}
				break;
			}
			break;
		case Type::o:
			switch (rotation) {
			case 0:
			case 1:
			case 2:
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = O1[i];
				}
				break;
			}
			break;
		case Type::j:
			switch (rotation) {
			case 0:
				for (int i = 0; i < 4; i++) {
					orientation[i] = J1[i];
				}
				break;
			case 1:
				for (int i = 0; i < 4; i++) {
					orientation[i] = J2[i];
				}
				break;
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = J3[i];
				}
				break;
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = J4[i];
				}
				break;
			}
			break;
		case Type::l:
			switch (rotation) {
			case 0:
				for (int i = 0; i < 4; i++) {
					orientation[i] = L1[i];
				}
				break;
			case 1:
				for (int i = 0; i < 4; i++) {
					orientation[i] = L2[i];
				}
				break;
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = L3[i];
				}
				break;
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = L4[i];
				}
				break;
			}
			break;
		case Type::i:
			switch (rotation) {
			case 0:
			case 2:
				for (int i = 0; i < 4; i++) {
					orientation[i] = I1[i];
				}
				break;
			case 1:
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = I2[i];
				}
				break;
			}
			break;
		case Type::n:
			switch (rotation) {
			case 0:
			case 1:
			case 2:
			case 3:
				for (int i = 0; i < 4; i++) {
					orientation[i] = n1[i];
				}
				break;
			}
			break;
		}
	}

	//rotate in a certain direction
	void Rotate(bool right, bool update = true) {
		//left or right
		if (!right) {
			rotation++;
			if (rotation > 3) { rotation = 0; }
		}
		else {
			rotation--;
			if (rotation < 0) { rotation = 3; }
		}

		//set the block orientations
		SetOrientation();

		//if the function is set to update, update the positions
		if (update) {
			//if there is collisions at the new rotation
			if (field[pos.X + orientation[0].X][pos.Y + orientation[0].Y] != 0 || PositionOutOfBounds(pos.X + orientation[0].X, pos.Y + orientation[0].Y) ||
				field[pos.X + orientation[1].X][pos.Y + orientation[1].Y] != 0 || PositionOutOfBounds(pos.X + orientation[1].X, pos.Y + orientation[1].Y) ||
				field[pos.X + orientation[2].X][pos.Y + orientation[2].Y] != 0 || PositionOutOfBounds(pos.X + orientation[2].X, pos.Y + orientation[2].Y) ||
				field[pos.X + orientation[3].X][pos.Y + orientation[3].Y] != 0 || PositionOutOfBounds(pos.X + orientation[3].X, pos.Y + orientation[3].Y)) {
				//rotate the opposite direction
				if (right) {
					Rotate(false, false);
				}
				else {
					Rotate(true, false);
				}
			} else {
				//if there is no collision, update to the new orientation
				Update();
			}
		}
	}

	//move the tetromino
	bool Move(int x, int y) {
		//make sure this tetromino exists
		if (!this) return true;

		//check if the position of any of the blocks is out of bounds
		if (PositionOutOfBounds(b1->pos.X + x, b1->pos.Y + y) ||
			PositionOutOfBounds(b2->pos.X + x, b2->pos.Y + y) ||
			PositionOutOfBounds(b3->pos.X + x, b3->pos.Y + y) ||
			PositionOutOfBounds(b4->pos.X + x, b4->pos.Y + y)) {
			if (y == 1)	{
				//bottom
				field[b1->pos.X][b1->pos.Y] = color;
				field[b2->pos.X][b2->pos.Y] = color;
				field[b3->pos.X][b3->pos.Y] = color;
				field[b4->pos.X][b4->pos.Y] = color;
				delete b1;
				delete b2;
				delete b3;
				delete b4;
				return false;
			}
			return true;
		} 
		//check if the position of any of the blocks are colliding
		if (field[b1->pos.X + x][b1->pos.Y + y] != 0 ||
			field[b2->pos.X + x][b2->pos.Y + y] != 0 ||
			field[b3->pos.X + x][b3->pos.Y + y] != 0 ||
			field[b4->pos.X + x][b4->pos.Y + y] != 0) {
			if (y == 1) {
				//bottom
				field[b1->pos.X][b1->pos.Y] = color;
				field[b2->pos.X][b2->pos.Y] = color;
				field[b3->pos.X][b3->pos.Y] = color;
				field[b4->pos.X][b4->pos.Y] = color;
				delete b1;
				delete b2;
				delete b3;
				delete b4;
				return false;
			}
			return true;
		}
		//move the mino
		pos.X += x;
		pos.Y += y;
		Update();
		return true;
	}

	//update the position of the blocks
	void Update() {
		b1->MoveTo(pos.X + orientation[0].X, pos.Y + orientation[0].Y);
		b2->MoveTo(pos.X + orientation[1].X, pos.Y + orientation[1].Y);
		b3->MoveTo(pos.X + orientation[2].X, pos.Y + orientation[2].Y);
		b4->MoveTo(pos.X + orientation[3].X, pos.Y + orientation[3].Y);
	}
};

int main() {

	//drop frame counts up to the dropframes, consider this the speed at which minos fall
	int dropFrame = 0;
	const int dropFrames = 3;

	//das = delayed auto shift
	//when held, after a the das frame delay, the pieces will move every 6 frames
	bool dasHeld = false;
	int dasFrame = 16;
	int dasFrames = 16;

	//count the frames
	double frame;
	clock_t start;
	start = clock();

	//count the seconds
	double second;
	clock_t secondstart;
	secondstart = clock();

	//set t to a pointer to a mino
	Tetromino* t = nullptr;

	//draw the edge of the field
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 21; j++)
		{
			if (i == 11 || i == 0 || j == 20) {
				DrawTextAtPosition(" ", 7 * 16 + 7, { (short)i, (short)j });
			}
		}
	}

	//make the cursor invisible
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = false; // set the cursor visibility
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	//main game loop
	while (true) {

		//second count
		second = (clock() - secondstart) / (double)CLOCKS_PER_SEC;
		if (second >= 1) {
			secondstart = clock();
		}
		//frame count
		frame = (clock() - start) / (double)CLOCKS_PER_SEC;
		DrawTextAtPosition(to_string(second/frame), 7, { 30, 2 });

		//frame update
		if (frame > (float)1 / (float)framerate)	{
			//spawn new tetromino if one does not exist
			if (t == nullptr) t = new Tetromino(5, 1, rand() % 7 + 1);

			//check key pressed X
			if (GetAsyncKeyState('X')) {
				//rotate the mino left
				if (!keyPressedRotateLeft) {
					keyPressedRotateLeft = true;
					t->Rotate(false);
				}
			} else {
				keyPressedRotateLeft = false;
			}

			//check key pressed Z
			if (GetAsyncKeyState('Z')) {
				//rotate the mino right
				if (!keyPressedRotateRight) {
					keyPressedRotateRight = true;
					t->Rotate(true);
				}
			} else {
				keyPressedRotateRight = false;
			}

			//check key pressed up arrow
			if (GetAsyncKeyState(VK_UP)) {
				//move the mino up (THIS SHOULD BE REMOVED)
				if (!keyPressedUp) {
					keyPressedUp = true;
					t->Move(0, -1);
				}
			} else {
				keyPressedUp = false;
			}
			
			//check key pressed down arrow
			if (GetAsyncKeyState(VK_DOWN)) {
				//move mino down 
				if (!keyPressedDown) {
					keyPressedDown = true;
					if (!t->Move(0, 1)) {
						delete t;
						t = nullptr;
					}
				}
			} else {
				keyPressedDown = false;
			}

			//check key pressed right arrow
			if (GetAsyncKeyState(VK_RIGHT)) {
				if (!keyPressedRight) {
					//set DAS
					dasHeld = true;
					keyPressedRight = true;
					//move mino
					if (!t->Move(1, 0)) {
						dasFrame = 0;
					}
				} else if (dasHeld && dasFrame < 0) {
					//also move mino automatically if das is still active and the frame count ends
					dasFrame = 6;
					if (!t->Move(1, 0)) {
						dasFrame = 0;
					}
				}
			}
			//check key pressed left arrow
			else if (GetAsyncKeyState(VK_LEFT)) {
				if (!keyPressedLeft) {
					//set DAS
					dasHeld = true;
					keyPressedLeft = true;
					if (!t->Move(-1, 0)) {
						dasFrame = 0;
					}
				} else if (dasHeld && dasFrame < 0) {
					//also move mino automatically if das is still active and the frame count ends
					dasFrame = 6;
					if (!t->Move(-1, 0)) {
						dasFrame = 0;
					}
				}
			}
			//if neither left or right is pressed
			else if (!GetAsyncKeyState(VK_LEFT) && !GetAsyncKeyState(VK_RIGHT)) {
				//stop DAS, reset DAS counter
				keyPressedLeft = false;
				keyPressedRight = false;
				dasHeld = false;
				dasFrame = 16;
			}
			else if (!GetAsyncKeyState(VK_LEFT)) {
				keyPressedLeft = false;
			}
			else if (!GetAsyncKeyState(VK_RIGHT)) {
				keyPressedRight = false;
			}

			//update the mino
			if (t != nullptr)
				t->Update();

			//increment DAS counter
			if (dasHeld) {
				dasFrame--;
			}

			//loop through field to draw
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 20; j++) {
					//if the position has something there, draw it
					if (field[i][j] != 0)
						DrawTextAtPosition(" ", field[i][j]*16+ field[i][j], { (short)i + 1, (short)j });
					else {
						//otherwise, if the tetromino's blocks are not there, put an empty space there
						if (t != nullptr) {
							if (!(t->b1->pos.X == i && t->b1->pos.Y == j) &&
								!(t->b2->pos.X == i && t->b2->pos.Y == j) &&
								!(t->b3->pos.X == i && t->b3->pos.Y == j) &&
								!(t->b4->pos.X == i && t->b4->pos.Y == j)) {
								DrawTextAtPosition(" ", field[i][j], { (short)i + 1, (short)j });
							}
						} else {
							DrawTextAtPosition(" ", field[i][j], { (short)i + 1, (short)j });
						}
					}
				}
			}

			//increment drop frames
			dropFrame++;
			if (dropFrame > dropFrames)	{
				if (!t->Move(0, 1))	{
					delete t;
					t = nullptr;
				}
				dropFrame = 0;
			}

			//check for line clears
			bool lineClear = true;
			//loop through rows
			for (int j = 0; j < 20; j++) {
				lineClear = true;
				//loop through collumns, if all are blocks, line clear
				for (int i = 0; i < 10; i++) {
					if (field[i][j] == 0)
						lineClear = false;
				}
				//clear line
				if (lineClear) {
					//set all to 0
					for (int i = 0; i < 10; i++) {
						field[i][j] = false;
					}
					//move all lines above down the equivalent
					for (int jj = j; jj > 1 ; jj--) {
						for (int i = 0; i < 10; i++) {
							field[i][jj] = field[i][jj-1];
						}
					}

				}
			}
			//count the time
			start = clock();
		}
	}
}
