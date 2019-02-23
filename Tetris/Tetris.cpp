//#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <Windows.h>
#include <algorithm>
#include <ctime>

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

bool keyPressedUp = false;
bool keyPressedDown = false;
bool keyPressedLeft = false;
bool keyPressedRight = false;
bool keyPressedRotateRight = false;
bool keyPressedRotateLeft = false;

CONST int framerate = 60;

int field[10][20];

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

enum Colors {
	c1 = 1,
	c2 = 9,
	c3 = 11
};

//add array of orientations
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

CONST COORD n1[4] = { { 0, 0 },{ 0, 0 },{ 0, 0 },{ 0, 0 } };

void DrawTextAtPosition(string text, int color, COORD position)
{
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

	SetConsoleCursorPosition(hConsole, { position.X , position.Y });
	SetConsoleTextAttribute(hConsole, color);
	cout << text;
	SetConsoleCursorPosition(hConsole, { 0 , 0 });
}

bool PositionOutOfBounds(int x, int y) {
	if (x < 0 || y < 0 || x > 9 || y > 19) return true;
	return false;
}

class Block
{
public:
	COORD pos;

	int color;

	Block();
	Block(short xx, short yy, int c)
	{
		pos = { xx, yy };
		color = c;
	}

	void MoveTo(int x, int y) {
		if (x != pos.X && y != pos.Y)
			DrawTextAtPosition(" ", 0, { pos.X + 1, pos.Y });
		pos.X = x;
		pos.Y = y;
		DrawTextAtPosition(" ", color*16+color, { pos.X + 1, pos.Y });
	}
};

class Tetromino {
public:
	COORD pos;

	int rotation;
	COORD orientation[4];

	int type;
	int color;

	Block* b1;
	Block* b2;
	Block* b3;
	Block* b4;

	Tetromino();
	Tetromino(short xx, short yy, int t) {
		pos = { xx, yy };
		type = t;
		switch (type) {
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
		rotation = 2;
		b1 = new Block(pos.X, pos.Y, color);
		b2 = new Block(pos.X, pos.Y, color);
		b3 = new Block(pos.X, pos.Y, color);
		b4 = new Block(pos.X, pos.Y, color);
		SetOrientation();
		Update();
		if (Collision())	{
			//die
		}
	}

	~Tetromino() { };

	bool Collision() {
		return (field[b1->pos.X][b1->pos.Y] != 0 ||
			field[b2->pos.X][b2->pos.Y] != 0 ||
			field[b3->pos.X][b3->pos.Y] != 0 ||
			field[b4->pos.X][b4->pos.Y] != 0);
	}

	void SetOrientation() {
		switch (type) {
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

	void Rotate(bool right, bool update = true) {
		if (!right) {
			rotation++;
			if (rotation > 3) { rotation = 0; }
		}
		else {
			rotation--;
			if (rotation < 0) { rotation = 3; }
		}

		SetOrientation();

		if (update) {
			if (field[pos.X + orientation[0].X][pos.Y + orientation[0].Y] != 0 || PositionOutOfBounds(pos.X + orientation[0].X, pos.Y + orientation[0].Y) ||
				field[pos.X + orientation[1].X][pos.Y + orientation[1].Y] != 0 || PositionOutOfBounds(pos.X + orientation[1].X, pos.Y + orientation[1].Y) ||
				field[pos.X + orientation[2].X][pos.Y + orientation[2].Y] != 0 || PositionOutOfBounds(pos.X + orientation[2].X, pos.Y + orientation[2].Y) ||
				field[pos.X + orientation[3].X][pos.Y + orientation[3].Y] != 0 || PositionOutOfBounds(pos.X + orientation[3].X, pos.Y + orientation[3].Y)) {

				if (right) {
					Rotate(false, false);
				}
				else {
					Rotate(true, false);
				}
			} else {
				Update();
			}
		}
	}

	bool Move(int x, int y) {
		if (!this) return true;
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
		pos.X += x;
		pos.Y += y;
		Update();
		return true;
	}

	void Update() {
		b1->MoveTo(pos.X + orientation[0].X, pos.Y + orientation[0].Y);
		b2->MoveTo(pos.X + orientation[1].X, pos.Y + orientation[1].Y);
		b3->MoveTo(pos.X + orientation[2].X, pos.Y + orientation[2].Y);
		b4->MoveTo(pos.X + orientation[3].X, pos.Y + orientation[3].Y);
	}
};

int main() {

	int dropFrame = 0;
	int dropFrames = 3;

	bool dasHeld = false;
	int dasFrame = 16;
	int dasFrames = 16;

	clock_t start;
	double frame;
	start = clock();

	Tetromino* t = nullptr;

	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 21; j++)
		{
			if (i == 11 || i == 0 || j == 20) {
				DrawTextAtPosition(" ", 7 * 16 + 7, { (short)i, (short)j });
			}
		}
	}


	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = false; // set the cursor visibility
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	double second;
	clock_t secondstart;
	secondstart = clock();

	while (true) {
		//tick
		second = (clock() - secondstart) / (double)CLOCKS_PER_SEC;
		if (second >= 1) {
			secondstart = clock();
		}

		frame = (clock() - start) / (double)CLOCKS_PER_SEC;
		DrawTextAtPosition(to_string(second/frame), 7, { 30, 2 });
		if (frame > (float)1 / (float)framerate)	{
			if (t == nullptr) t = new Tetromino(5, 1, rand() % 7 + 1);

			int c = 0;

			if (GetAsyncKeyState('X')) {
				if (!keyPressedRotateLeft) {
					keyPressedRotateLeft = true;
					t->Rotate(false);
				}
			}
			else {
				keyPressedRotateLeft = false;
			}

			if (GetAsyncKeyState('Z')) {
				if (!keyPressedRotateRight) {
					keyPressedRotateRight = true;
					t->Rotate(true);
				}
			}
			else {
				keyPressedRotateRight = false;
			}

			if (GetAsyncKeyState(VK_UP)) {
				if (!keyPressedUp) {
					keyPressedUp = true;
					t->Move(0, -1);
				}
			}
			else {
				keyPressedUp = false;
			}

			if (GetAsyncKeyState(VK_DOWN)) {
				if (!keyPressedDown) {
					keyPressedDown = true;
					if (!t->Move(0, 1)) {
						delete t;
						t = nullptr;
					}
				}
			}
			else {
				keyPressedDown = false;
			}

			if (GetAsyncKeyState(VK_RIGHT)) {
				if (!keyPressedRight) {
					dasHeld = true;
					keyPressedRight = true;
					if (!t->Move(1, 0)) {
						dasFrame = 0;
					}
				}
				else if (dasHeld && dasFrame < 0) {
					dasFrame = 6;
					if (!t->Move(1, 0)) {
						dasFrame = 0;
					}
				}
			}
			else if (GetAsyncKeyState(VK_LEFT)) {
				if (!keyPressedLeft) {
					dasHeld = true;
					keyPressedLeft = true;
					if (!t->Move(-1, 0)) {
						dasFrame = 0;
					}
				}
				else if (dasHeld && dasFrame < 0) {
					dasFrame = 6;
					if (!t->Move(-1, 0)) {
						dasFrame = 0;
					}
				}
			}
			else if (!GetAsyncKeyState(VK_LEFT) && !GetAsyncKeyState(VK_RIGHT)) {
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

			if (t != nullptr)
				t->Update();

			if (dasHeld) {
				dasFrame--;
			}

			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 20; j++) {
					if (field[i][j] != 0)
						DrawTextAtPosition(" ", field[i][j]*16+ field[i][j], { (short)i + 1, (short)j });
					else {
						if (t != nullptr) {
							if (!(t->b1->pos.X == i && t->b1->pos.Y == j) &&
								!(t->b2->pos.X == i && t->b2->pos.Y == j) &&
								!(t->b3->pos.X == i && t->b3->pos.Y == j) &&
								!(t->b4->pos.X == i && t->b4->pos.Y == j)) {
								DrawTextAtPosition(" ", field[i][j], { (short)i + 1, (short)j });
							}
						}
						else {
							DrawTextAtPosition(" ", field[i][j], { (short)i + 1, (short)j });
						}
					}
				}
			}

			dropFrame++;
			if (dropFrame > dropFrames)	{
				if (!t->Move(0, 1))	{
					delete t;
					t = nullptr;
				}
				dropFrame = 0;
			}

			bool lineClear = true;
			for (int j = 0; j < 20; j++) {
				lineClear = true;
				for (int i = 0; i < 10; i++) {
					if (field[i][j] == 0)
						lineClear = false;
				}
				if (lineClear) {
					for (int i = 0; i < 10; i++) {
						field[i][j] = false;
					}

					for (int jj = j; jj > 1 ; jj--) {
						for (int i = 0; i < 10; i++) {
							field[i][jj] = field[i][jj-1];
						}
					}

				}
			}
			start = clock();
		}
	}
}