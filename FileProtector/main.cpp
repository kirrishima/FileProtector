#define NOMINMAX
#include "stdafx.h"
#include "imghider.h"
#include <VideoEncryptor.h>
#include <filesystem>
#include <limits>
#include <regex>
#include <iostream>
#include <memory>

std::unique_ptr<VideoEncryptor> VE = std::make_unique<VideoEncryptor>("Force_AKA_Moonglow_Qpro_I_Love_Sadar_baby_Goat", "VIDS", "Input", "Corrupted", "Fixed", 4);

const std::string imagesBaseDirectory = "IMG";
const std::string imagesSaveFromPath = (std::filesystem::path(imagesBaseDirectory) / "Upload").string();
const std::string imagesSaveToPath = (std::filesystem::path(imagesBaseDirectory) / "Saves").string();
const std::string binaryPath = (std::filesystem::path(imagesSaveToPath) / "data.bin").string();
const std::string imagesRecoveredDirectory = (std::filesystem::path(imagesBaseDirectory) / "Recovered").string();
const std::string hashFilePath = (std::filesystem::path(imagesSaveToPath) / "hash.txt").string();

const std::string videoBaseDirectory = VE->getBaseDirectory();
const std::string videoInputPath = (std::filesystem::path(videoBaseDirectory) / VE->getInputFolder()).string();
const std::string videoEncryptedPath = (std::filesystem::path(videoBaseDirectory) / VE->getEncryptedFolder()).string();
const std::string videoRecoveredDirectory = (std::filesystem::path(videoBaseDirectory) / VE->getDecryptedFolder()).string();

int ConsoleDefaultHeight = GetConsoleWindowHeight();
#define DISABLE_PSWD
bool authenticateUser() {
#ifndef DISABLE_PSWD
	std::cout << "������� ������: ";
	std::string pswd;
	std::getline(std::cin, pswd);
	if (pswd != "735812") {

		std::cout << "����� ����������!\n������� ������:\n������ ���������:\nhttps://open.spotify.com/track/5ucIAerBlXCrES9RNEGNzH?si=30a9324a1cd34b31\n";
		system("pause");
		return false;
	}
#endif
	return true;
}

void initializeDirectories(const std::vector<std::string>& paths, bool verbose = false) {
	printColoredMessage("��������� ��� ���������� �����, ���� �� ���. . . \n\n", CONSOLE_GREEN);
	imghider::checkAndCreatePaths({ paths, verbose });
}

void displayMainMenu();
void showDeletionMenu();
void handleUserInput(char userInput, const std::vector<std::string>& paths);
void printHelp();


int main() {
	std::setlocale(LC_ALL, "ru_RU");

	ResizeConsole(36, 120, 10000, 120);

	if (!authenticateUser()) {
		return 1;
	}

	//SetCursorPosition(0, cursorPos.Y);
	SetCursorPosition(0, 0);
	printColoredMessage("�� �������� ����� " + binaryPath + " � " + videoEncryptedPath + " � �� ���������� �� ��������� ������ ����������\n", CONSOLE_RED);

	std::vector<std::string> paths = { imagesSaveFromPath, imagesSaveToPath, binaryPath, imagesRecoveredDirectory, hashFilePath,
		videoInputPath, videoEncryptedPath, videoRecoveredDirectory };

	initializeDirectories(paths);

	uchar userInput;
	std::cout << std::endl;
	do {
		displayMainMenu();
		std::cin >> userInput;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		handleUserInput(userInput, paths);
	} while (true);

	system("pause");
	return 0;
}

void handleUserInput(char userInput, const std::vector<std::string>& paths) {
	switch (userInput) {
	case '1':
		std::cout << "\n";
		initializeDirectories(paths, true);
		break;
	case '2':
		imghider::saveImagesToBinary(imagesSaveFromPath, binaryPath, hashFilePath);
		std::cout << std::endl;
		break;
	case '3':
		imghider::loadImagesFromBinary(binaryPath, imagesRecoveredDirectory);
		std::cout << std::endl;
		break;
	case '4':
		VE->encryptMp4();
		break;
	case '5':
		VE->decryptMp4();
		break;
	case '6':
		VE->setShouldEncryptDecryptedFolder(true);
		break;
	case '7':
		showDeletionMenu();
		break;
	case '8':
		printHelp();
		break;
	case '0':
	case 'q':
		exit(EXIT_SUCCESS);
	default:
		printColoredMessage("������������ ����", CONSOLE_RED);
		break;
	}
}

void printHelp() {
	std::cout << "\n\n";
	printCentered("��������� ��� ���������� � �������� ����������� � �����.\n", CONSOLE_GREEN);

	printColoredMessage("��� ��������� ��������� ���:", CONSOLE_DARK_CYAN);
	std::cout << "1. ��������� ����������� � �������� ����." << std::endl;
	std::cout << "2. ��������� ����������� �� ��������� �����." << std::endl;
	std::cout << "3. ����������� � ������������ ����������." << std::endl;
	std::cout << "4. ������� ����� � �����, ��������� � ��������� ���������� � ��������." << std::endl;
	std::cout << std::endl;

	printColoredMessage("������������ ���� � �����:", CONSOLE_DARK_CYAN);

	printColoredMessage("\n��� �����������:", CONSOLE_DARK_YELLOW);
	std::cout << " - ������� ��� �������� �����������: " << imagesSaveFromPath << std::endl;
	std::cout << " - ������� ��� ���������� ����������� � �������� �������: " << binaryPath << std::endl;
	std::cout << " - ������� ��� �������������� �����������: " << imagesRecoveredDirectory << std::endl;
	std::cout << " - ������� ��� ���������� ���� �����������: " << hashFilePath << std::endl;
	printColoredMessage("\n��� �����:", CONSOLE_DARK_YELLOW);
	std::cout << " - ������� ��� ������� �����������: " << videoInputPath << std::endl;
	std::cout << " - ������� ��� ���������� ������������� �����������: " << videoEncryptedPath << std::endl;
	std::cout << " - ������� ��� �������������� �������������� �����������: " << videoRecoveredDirectory << std::endl;
	std::cout << std::endl;

	printColoredMessage("��� ������������:", CONSOLE_DARK_CYAN);

	std::cout << " - ���������� ������ ����� ��� ������, ���������� ������������ ����� ������ �� ������� ����" << std::endl;
	std::cout << std::endl;

	printColoredMessage("��������:", CONSOLE_DARK_RED);
	std::cout << " - ������ �� " << binaryPath << " �� ��������� ��� �������������� �����������, ������� �������� ��������� ��������������� ����������� �� �����\n";
	std::cout << " - ������������� ���������� �� " << videoEncryptedPath << " �� ��������� ��� �������������, ������� �������� ��������� �� ����� �� �����\n";
	std::cout << " - ��� ������ 6 ������ �������� ���� - ��������� ���������� ����� �� ����� � ��������������� ����� - ������ ��������� ��������� ������ � ���� ��� � �� ����� ��������� ��� ����������� ��������\n";
	std::cout << " - �������� ����� � ������ ����������. ������ ��������� ��� ������ ��������������� ������� ����." << std::endl;
	std::cout << " - �� �������� �����, ���������� ������ �����, ����� ���: " << binaryPath << " � " << videoEncryptedPath << " �� ��������� ������ ���������� ������." << std::endl;
	std::cout << std::endl;

	SET_CONSOLE_DARK_CYAN;
	system("pause");
	SET_CONSOLE_DEFAULT;
}

void showDeletionMenu() {
	bool exitFlag = false;
	std::regex numberPattern("^[0-9]+$");
	printColoredMessage("\n���� �������� ������ � �����", CONSOLE_CYAN);

	while (!exitFlag) {
		std::cout << "\n0, q - ������� � ������� ����\n";
		printColoredMessage("----- ����������� -----", CONSOLE_CYAN);
		std::cout << "1) ��������� ��� ����� " << imagesBaseDirectory << std::endl;
		std::cout << "2) ������� ��������������� ����������� �� " << imagesRecoveredDirectory << std::endl;
		std::cout << "3) ������� ����������� ����������� �� " << imagesSaveToPath << std::endl;
		std::cout << "4) ������� ������������� ����������� �� " << imagesSaveFromPath << std::endl;
		printColoredMessage("----- ����� -----", CONSOLE_CYAN);
		std::cout << "5) ������� �� �� " << videoBaseDirectory << std::endl;
		std::cout << "6) ������� ��������������� ����� �� " << videoRecoveredDirectory << std::endl;
		std::cout << "7) ������� ������������� ����� �� " << videoEncryptedPath << std::endl;
		std::cout << "8) ������� ������������� ����� �� " << videoInputPath << std::endl;
		printColoredMessage("----- ����������� + ����� -----", CONSOLE_CYAN);
		std::cout << "9) ������� �� �� " << imagesBaseDirectory + " � " + videoBaseDirectory << std::endl;
		std::cout << "10) ������� ��������������� ����������� � ����� �� " << imagesRecoveredDirectory + " � " + videoRecoveredDirectory << std::endl;
		std::cout << "11) ������� ����������� ����������� �� " << imagesSaveToPath + " � ������������� ����� �� " + videoEncryptedPath << std::endl;
		std::cout << "12) ������� ������������� ����������� � ����� �� " << imagesSaveFromPath + " � " + videoInputPath << std::endl;

		printColoredMessage("\n������� ��� �����: ", CONSOLE_BLUE, "");

		std::string userInput;
		std::getline(std::cin, userInput);

		if (userInput == "0" || userInput == "q") {
			exitFlag = true;
			continue;
		}

		if (!std::regex_match(userInput, numberPattern)) {
			printColoredMessage("������������ ����", CONSOLE_RED);
			continue;
		}

		int choice = -1;
		try {
			size_t pos;
			choice = std::stoi(userInput, &pos);
			if (pos != userInput.length()) {
				printColoredMessage("������������ ����", CONSOLE_RED);
				continue;
			}
		}
		catch (std::invalid_argument&) {
			printColoredMessage("������������ ����", CONSOLE_RED);
			continue;
		}
		catch (std::out_of_range&) {
			printColoredMessage("������������ ����", CONSOLE_RED);
			continue;
		}

		printColoredMessage("\n��������: ", CONSOLE_RED, "");
		printColoredMessage(" ��� �������� ������ ����� �������� � ������ ����� ������� ", CONSOLE_CYAN, "");
		printColoredMessage("������������", CONSOLE_RED, "");

		char confirm;
		do {
			printColoredMessage("\n������� ���������� (y/n)? ", CONSOLE_CYAN, "");
			std::cin >> confirm;
			confirm = std::tolower(confirm);
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (confirm != 'y' && confirm != 'n');

		if (confirm == 'n') {
			printColoredMessage("�������� ���� ��������", CONSOLE_YELLOW);
			continue;
		}

		std::cout << std::endl;

		switch (choice) {
		case 1:
			imghider::clearDirectory(imagesBaseDirectory, true);
			break;
		case 2:
			imghider::clearDirectory(imagesRecoveredDirectory);
			break;
		case 3:
			imghider::clearDirectory(imagesSaveToPath);
			break;
		case 4:
			imghider::clearDirectory(imagesSaveFromPath);
			break;
		case 5:
			imghider::clearDirectory(videoBaseDirectory, true);
			break;
		case 6:
			imghider::clearDirectory(videoRecoveredDirectory);
			break;
		case 7:
			imghider::clearDirectory(videoEncryptedPath);
			break;
		case 8:
			imghider::clearDirectory(videoInputPath);
			break;
		case 9:
			imghider::clearDirectory(imagesBaseDirectory, true);
			std::cout << std::endl;
			imghider::clearDirectory(videoBaseDirectory, true);
			break;
		case 10:
			imghider::clearDirectory(imagesRecoveredDirectory);
			std::cout << std::endl;
			imghider::clearDirectory(videoRecoveredDirectory);
			break;
		case 11:
			imghider::clearDirectory(imagesSaveToPath);
			std::cout << std::endl;
			imghider::clearDirectory(videoEncryptedPath);
			break;
		case 12:
			imghider::clearDirectory(imagesSaveFromPath);
			std::cout << std::endl;
			imghider::clearDirectory(videoInputPath);
			break;
		default:
			std::cout << "������������ ����\n";
			break;
		}
	}
	printColoredMessage("\n������� � ������� ����. . .", CONSOLE_CYAN);
}

void displayMainMenu() {
	printColoredMessage("\n������� ����:", CONSOLE_CYAN);
	std::cout << "0, q - �����\n";
	std::cout << "1) ������� ��� ����������� ����� � �����\n";
	std::cout << "2) ��������� ����������� �� " << imagesSaveFromPath << std::endl;
	std::cout << "3) ��������� ����������� ����������� �� " << binaryPath << std::endl;
	std::cout << "4) ����������� ����� �� " << videoInputPath << std::endl;
	std::cout << "5) ������������ ����� �� " << videoEncryptedPath << std::endl;
	std::cout << "6) ����� ��������� �������������� ����� �� " << videoInputPath << " ��� ������ 4 ������ � ��������� � " << videoEncryptedPath << " (�� ��������� ��� �� ���������, ������������� ����� ��� ����������� �� ���������!!)" << std::endl;
	std::cout << "7) ���� �������� ������ � �����\n";
	std::cout << "8) ������� �� ���������\n";
	printColoredMessage("\n������� ��� �����: ", CONSOLE_BLUE, "");
}