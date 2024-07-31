#include "stdafx.h"
#include "imghider.h"
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <hashing.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace imghider {

	std::string getUniqueFilename(const std::string& originalFilename) {
		try {
			std::string extension = originalFilename.substr(originalFilename.find_last_of('.'));
			std::string baseFilename = originalFilename.substr(0, originalFilename.find_last_of('.'));

			std::time_t now = std::time(nullptr);
			std::tm localTime;
			localtime_s(&localTime, &now);

			std::ostringstream oss;
			oss << baseFilename << "_"
				<< std::put_time(&localTime, "%Y%m%d_%H%M%S")
				<< extension;

			return oss.str();
		}
		catch (const std::exception& e) {
			printColoredMessage("Ошибка при генерации уникального имени файла: " + std::string(e.what()), CONSOLE_RED);
			return originalFilename; // fallback to original filename in case of error
		}
	}

	bool resolveDuplicate(
		const std::string& directoryPath,
		const std::string& binaryPath,
		const std::string& imagePath,
		const std::string& filename,
		const std::string& fileRelPath,
		std::string& newFilename,
		std::string& fileHash,
		const cv::Mat& image
	) {
		try {
			bool flagExit = false;
			SET_CONSOLE_RED;
			std::cout << "\nФайл с названием \"" << filename << "\" уже есть в " << binaryPath << std::endl;

			do {
				printColoredMessage("\n1) Просмотреть сохраненный файл " + binaryPath + '\\' + filename, CONSOLE_CYAN);
				printColoredMessage("2) Просмотреть файл " + imagePath, CONSOLE_YELLOW);
				printColoredMessage("3) Сохранить файл с другим названием.", CONSOLE_CYAN);
				printColoredMessage("4) Пропустить этот файл.\n", CONSOLE_YELLOW);

				SET_CONSOLE_MAGENTA;
				int x;
				std::cin >> x;
				std::cin.ignore();
				SET_CONSOLE_DEFAULT;

				switch (x) {
				case 1:
					printColoredMessage("\nПоиск " + filename + " в " + binaryPath + ". . .", CONSOLE_BLUE);
					findAndDisplayImage(binaryPath, fileRelPath);
					break;
				case 2:
					displayImage(image, imagePath);
					break;
				case 3: {

					newFilename = getUniqueFilename(filename);
					fileHash = safe_hashing::sha256(newFilename);
					flagExit = true;
					break;
				}
				case 4:
					SET_CONSOLE_DEFAULT;
					std::cout << "\nФайл " + imagePath + " остался в " + directoryPath + " и не был сохранен в " + binaryPath << std::endl;
					return true;
				default:
					printColoredMessage("Некорректный ввод: " + std::to_string(x) + ". Повторите попытку\n", CONSOLE_RED);
				}
			} while (!flagExit);
		}
		catch (const std::exception& e) {
			printColoredMessage("Ошибка при разрешении дубликата: " + std::string(e.what()), CONSOLE_RED);
			return true; // return true to skip the file in case of error
		}
		return false;
	}

	// Функция для разделения строки по заданному разделителю
	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	// Функция для получения последнего элемента после разделения строки
	std::string get_last_split_element(const std::string& str, char delimiter) {
		std::vector<std::string> tokens = split(str, delimiter);
		if (!tokens.empty()) {
			return tokens.back(); // Получаем последний элемент
		}
		return "";
	}


} //imghider