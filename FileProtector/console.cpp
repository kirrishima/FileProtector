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
	// �������� ���������� �������
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return -1; // ���������� -1 � ������ ������
	}

	// ��������� ��� �������� ���������� � ������ ������
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "�� ������� �������� ���������� � ������ ������.\n";
		return -1; // ���������� -1 � ������ ������
	}

	// ������ ���� � �������
	int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return windowHeight;
}

int GetConsoleWidth() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return -1;
	}

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "�� ������� �������� ���������� � ������ ������.\n";
		return -1;
	}

	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void ResizeConsole(int windowHeight, int bufferHeight) {
	// �������� ���������� ���� �������
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return;
	}

	// �������� ������� ���������� � ������ ������
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		std::cerr << "�� ������� �������� ���������� � ������ ������.\n";
		return;
	}

	int width = csbi.dwSize.X; // ������ �������� �������

	// ������������� ����� ������ ������ ������
	COORD newSize;
	newSize.X = width;
	newSize.Y = bufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, newSize)) {
		std::cerr << "�� ������� ���������� ������ ������ ������.\n";
		return;
	}

	// ������������� ����� ������ ���� �������
	SMALL_RECT windowSize;
	windowSize.Left = 0;
	windowSize.Top = 0;
	windowSize.Right = width - 1;
	windowSize.Bottom = windowHeight - 1;
	if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
		std::cerr << "�� ������� ���������� ������� ���� �������.\n";
		return;
	}
}

void ResizeConsole(int windowHeight, int windowWidth, int bufferHeight, int bufferWidth) {
	// �������� ���������� ���� �������
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return;
	}

	// ������������� ����� ������ ������ ������
	COORD newSize;
	newSize.X = bufferWidth;
	newSize.Y = bufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, newSize)) {
		std::cerr << "�� ������� ���������� ������ ������ ������.\n";
		return;
	}

	// ������������� ����� ������ ���� �������
	SMALL_RECT windowSize;
	windowSize.Left = 0;
	windowSize.Top = 0;
	windowSize.Right = windowWidth - 1;
	windowSize.Bottom = windowHeight - 1;
	if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
		std::cerr << "�� ������� ���������� ������� ���� �������.\n";
		return;
	}
}

void SetConsoleFontSize(int fontSizeX, int fontSizeY) {
	// �������� ���������� �������
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return;
	}

	// �������� ������� �������� ������
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	if (!GetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "�� ������� �������� ������� �������� ������.\n";
		return;
	}

	// ������������� ����� ������ ������
	cfi.dwFontSize.X = fontSizeX;
	cfi.dwFontSize.Y = fontSizeY;
	if (!SetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "�� ������� ���������� ����� ������ ������.\n";
		return;
	}
}

std::tuple<short, short> GetConsoleFontSize() {
	// �������� ���������� �������
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		std::cerr << "�� ������� �������� ���������� �������.\n";
		return std::make_tuple(-1, -1);
	}

	// �������� ������� �������� ������
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);

	if (!GetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
		std::cerr << "�� ������� �������� ������� �������� ������.\n";
		return std::make_tuple(-1, -1);
	}

	return std::make_tuple(cfi.dwFontSize.X, cfi.dwFontSize.Y);
}
