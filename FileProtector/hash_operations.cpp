#include "stdafx.h"
#include <cstdint>
#include <ctime>
#include <iomanip>

namespace imghider {
	bool isNameHashInFile(const std::string& hashFilePath, const std::string& fileHash) {
		try {
			std::ifstream hashFile(hashFilePath);
			if (!hashFile) {
				printColoredMessage("Ошибка при поиске хэша: не удалось открыть файл " + hashFilePath, CONSOLE_RED);
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
			printColoredMessage("Ошибка при чтении файла хэша: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при поиске хэша в файле " + hashFilePath, CONSOLE_RED);
		}

		return false;
	}

	void addNameHashToFile(const std::string& hashFilePath, const std::string& hashValue) {
		try {
			std::ofstream hashFile(hashFilePath, std::ios::out | std::ios::app);
			if (!hashFile) {
				printColoredMessage("Ошибка при записи хэша: не удалось открыть файл " + hashFilePath, CONSOLE_RED);
				return;
			}
			hashFile << hashValue << std::endl;
		}
		catch (const std::ofstream::failure& e) {
			printColoredMessage("Ошибка при записи в файл хэша: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при записи хэша в файл " + hashFilePath, CONSOLE_RED);
		}
	}


}