#pragma once
#define NOMINMAX
#include <Windows.h>
#include <string>


#define PRINT_ALL_CODES_AND_COLORS 	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);\
for (int codeId = 0; codeId < 255; codeId++)\
{\
	SetConsoleTextAttribute(hConsole, k);\
	std::cout <<"Color code: "<< k << ". I want to be nice today!";\
	SetConsoleTextAttribute(hConsole, defaultAttributes);\
	std::cout << std::endl;\
}

#define CONSOLE_BLACK           0
#define CONSOLE_DARK_BLUE       1
#define CONSOLE_DARK_GREEN      2
#define CONSOLE_DARK_CYAN       3
#define CONSOLE_DARK_RED        4
#define CONSOLE_DARK_MAGENTA    5
#define CONSOLE_DARK_YELLOW     6
#define CONSOLE_DARK_GRAY       7
#define CONSOLE_GRAY            8
#define CONSOLE_BLUE            9
#define CONSOLE_GREEN           10
#define CONSOLE_CYAN            11
#define CONSOLE_RED             12
#define CONSOLE_MAGENTA         13
#define CONSOLE_YELLOW          14
#define CONSOLE_WHITE           15

#define CONSOLE_DEFAULT         defaultAttributes

// Макросы для установки цвета текста на консоли
#define SET_CONSOLE_BLACK          SetConsoleColor(CONSOLE_BLACK)
#define SET_CONSOLE_DARK_BLUE      SetConsoleColor(CONSOLE_DARK_BLUE)
#define SET_CONSOLE_DARK_GREEN     SetConsoleColor(CONSOLE_DARK_GREEN)
#define SET_CONSOLE_DARK_CYAN      SetConsoleColor(CONSOLE_DARK_CYAN)
#define SET_CONSOLE_DARK_RED       SetConsoleColor(CONSOLE_DARK_RED)
#define SET_CONSOLE_DARK_MAGENTA   SetConsoleColor(CONSOLE_DARK_MAGENTA)
#define SET_CONSOLE_DARK_YELLOW    SetConsoleColor(CONSOLE_DARK_YELLOW)
#define SET_CONSOLE_DARK_GRAY      SetConsoleColor(CONSOLE_DARK_GRAY)
#define SET_CONSOLE_GRAY           SetConsoleColor(CONSOLE_GRAY)
#define SET_CONSOLE_GREEN          SetConsoleColor(CONSOLE_GREEN)
#define SET_CONSOLE_RED            SetConsoleColor(CONSOLE_RED)
#define SET_CONSOLE_BLUE           SetConsoleColor(CONSOLE_BLUE)
#define SET_CONSOLE_CYAN           SetConsoleColor(CONSOLE_CYAN)
#define SET_CONSOLE_MAGENTA        SetConsoleColor(CONSOLE_MAGENTA)
#define SET_CONSOLE_YELLOW         SetConsoleColor(CONSOLE_YELLOW)
#define SET_CONSOLE_WHITE          SetConsoleColor(CONSOLE_WHITE)
#define SET_CONSOLE_DEFAULT        SetConsoleColor(CONSOLE_DEFAULT)

// Методы для работы с консолью
extern void SetConsoleColor(int color);
extern WORD GetConsoleTextAttribute();
extern void SetCursorPosition(int x, int y);

static WORD defaultAttributes = GetConsoleTextAttribute();

COORD getCursorposition();

void printColoredMessage(const std::string& message, WORD outputColor, std::string end = "\n");

void printCentered(const std::string& text, WORD outputColor = CONSOLE_DEFAULT, std::string end = "\n");

int GetConsoleWindowHeight();

int GetConsoleWidth();

void ResizeConsole(int windowHeight, int bufferHeight);
//void ResizeConsole(int windowHeight, int windowWidth, int bufferHeight, int bufferWidth);

void ResizeConsole(int windowHeight, int windowWidth, int bufferHeight, int bufferWidth);

void SetConsoleFontSize(int fontSizeX, int fontSizeY);

std::tuple<short, short> GetConsoleFontSize();
//#define DISABLE_PSWD
