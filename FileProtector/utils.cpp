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
			printColoredMessage("������ ��� ��������� ����������� ����� �����: " + std::string(e.what()), CONSOLE_RED);
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
			std::cout << "\n���� � ��������� \"" << filename << "\" ��� ���� � " << binaryPath << std::endl;

			do {
				printColoredMessage("\n1) ����������� ����������� ���� " + binaryPath + '\\' + filename, CONSOLE_CYAN);
				printColoredMessage("2) ����������� ���� " + imagePath, CONSOLE_YELLOW);
				printColoredMessage("3) ��������� ���� � ������ ���������.", CONSOLE_CYAN);
				printColoredMessage("4) ���������� ���� ����.\n", CONSOLE_YELLOW);

				SET_CONSOLE_MAGENTA;
				int x;
				std::cin >> x;
				std::cin.ignore();
				SET_CONSOLE_DEFAULT;

				switch (x) {
				case 1:
					printColoredMessage("\n����� " + filename + " � " + binaryPath + ". . .", CONSOLE_BLUE);
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
					std::cout << "\n���� " + imagePath + " ������� � " + directoryPath + " � �� ��� �������� � " + binaryPath << std::endl;
					return true;
				default:
					printColoredMessage("������������ ����: " + std::to_string(x) + ". ��������� �������\n", CONSOLE_RED);
				}
			} while (!flagExit);
		}
		catch (const std::exception& e) {
			printColoredMessage("������ ��� ���������� ���������: " + std::string(e.what()), CONSOLE_RED);
			return true; // return true to skip the file in case of error
		}
		return false;
	}

	// ������� ��� ���������� ������ �� ��������� �����������
	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	// ������� ��� ��������� ���������� �������� ����� ���������� ������
	std::string get_last_split_element(const std::string& str, char delimiter) {
		std::vector<std::string> tokens = split(str, delimiter);
		if (!tokens.empty()) {
			return tokens.back(); // �������� ��������� �������
		}
		return "";
	}


} //imghider