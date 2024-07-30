#include "stdafx.h"
#include "console.h"

void SetConsoleColor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

WORD GetConsoleTextAttribute() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(hConsole, &bufferInfo);
	return bufferInfo.wAttributes;
}

void SetCursorPosition(int x, int y) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
	SetConsoleCursorPosition(hConsole, coord);
}

void printColoredMessage(const std::string& message, WORD outputColor, std::string end)
{
	SetConsoleColor(outputColor);
	std::cout << message;
	SetConsoleColor(CONSOLE_DEFAULT);
	std::cout << end;
}

int getConsoleWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	return columns;
}

void printCentered(const std::string& text, WORD outputColor, std::string end) {
	int consoleWidth = getConsoleWidth();
	int textLength = text.length();
	int padding = (consoleWidth - textLength) / 2;

	for (int i = 0; i < padding; i++) {
		std::cout << ' ';
	}

	printColoredMessage(text, outputColor, end);
}


int GetConsoleWindowHeight() {
	// Получаем дескриптор консоли
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return -1; // Возвращаем -1 в случае ошибки
	}

	// Структура для хранения информации о буфере экрана
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "Не удалось получить информацию о буфере экрана.\n";
		return -1; // Возвращаем -1 в случае ошибки
	}

	// Высота окна в строках
	int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return windowHeight;
}

int GetConsoleWidth() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return -1;
	}

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "Не удалось получить информацию о буфере экрана.\n";
		return -1;
	}

	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void ResizeConsole(int windowHeight, int bufferHeight) {
	// Получаем дескриптор окна консоли
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return;
	}

	// Получаем текущую информацию о буфере экрана
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "Не удалось получить информацию о буфере экрана.\n";
		return;
	}

	int width = csbi.dwSize.X; // Ширина остается прежней

	// Устанавливаем новый размер буфера экрана
	COORD newSize;
	newSize.X = width;
	newSize.Y = bufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, newSize)) {
		std::cerr << "Не удалось установить размер буфера экрана.\n";
		return;
	}

	// Устанавливаем новый размер окна консоли
	SMALL_RECT windowSize;
	windowSize.Left = 0;
	windowSize.Top = 0;
	windowSize.Right = width - 1;
	windowSize.Bottom = windowHeight - 1;
	if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
		std::cerr << "Не удалось установить размеры окна консоли.\n";
		return;
	}
}

void ResizeConsole(int windowHeight, int windowWidth, int bufferHeight, int bufferWidth) {
	// Получаем дескриптор окна консоли
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return;
	}

	// Устанавливаем новый размер буфера экрана
	COORD newSize;
	newSize.X = bufferWidth;
	newSize.Y = bufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, newSize)) {
		std::cerr << "Не удалось установить размер буфера экрана.\n";
		return;
	}

	// Устанавливаем новый размер окна консоли
	SMALL_RECT windowSize;
	windowSize.Left = 0;
	windowSize.Top = 0;
	windowSize.Right = windowWidth - 1;
	windowSize.Bottom = windowHeight - 1;
	if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
		std::cerr << "Не удалось установить размеры окна консоли.\n";
		return;
	}
}

void SetConsoleFontSize(int fontSizeX, int fontSizeY) {
	// Получаем дескриптор консоли
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return;
	}

	// Получаем текущие свойства шрифта
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	if (!GetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "Не удалось получить текущие свойства шрифта.\n";
		return;
	}

	// Устанавливаем новый размер шрифта
	cfi.dwFontSize.X = fontSizeX;
	cfi.dwFontSize.Y = fontSizeY;
	if (!SetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "Не удалось установить новый размер шрифта.\n";
		return;
	}
}

std::tuple<short, short> GetConsoleFontSize() {
	// Получаем дескриптор консоли
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось получить дескриптор консоли.\n";
		return std::make_tuple(-1, -1);
	}

	// Получаем текущие свойства шрифта
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);

	if (!GetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "Не удалось получить текущие свойства шрифта.\n";
		return std::make_tuple(-1, -1);
	}

	return std::make_tuple(cfi.dwFontSize.X, cfi.dwFontSize.Y);
}
