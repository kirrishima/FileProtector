#include "stdafx.h"
#include "imghider.h"
#include <cstdint>
#include <ctime>
#include <iomanip>

namespace imghider {
	bool isNameHashInFile(const std::string& hashFilePath, const std::string& fileHash) {
		try {
			std::ifstream hashFile(hashFilePath);
			if (!hashFile) {
				printColoredMessage("������ ��� ������ ����: �� ������� ������� ���� " + hashFilePath, CONSOLE_RED);
				return false;
			}

			std::string storedHash;
			while (std::getline(hashFile, storedHash)) {
				if (storedHash == fileHash) {
					return true;
				}
			}
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("������ ��� ������ ����� ����: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������ ��� ������ ���� � ����� " + hashFilePath, CONSOLE_RED);
		}

		return false;
	}

	void addNameHashToFile(const std::string& hashFilePath, const std::string& hashValue) {
		try {
			std::ofstream hashFile(hashFilePath, std::ios::out | std::ios::app);
			if (!hashFile) {
				printColoredMessage("������ ��� ������ ����: �� ������� ������� ���� " + hashFilePath, CONSOLE_RED);
				return;
			}
			hashFile << hashValue << std::endl;
		}
		catch (const std::ofstream::failure& e) {
			printColoredMessage("������ ��� ������ � ���� ����: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������ ��� ������ ���� � ���� " + hashFilePath, CONSOLE_RED);
		}
	}


}