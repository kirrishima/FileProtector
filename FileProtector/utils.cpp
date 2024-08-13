#include "stdafx.h"
#include "opencv2/core/mat.hpp"
#include <ctime>
#include <hashing.h>
#include <iomanip>
#include <limits>

namespace fs = std::filesystem;

namespace imghider {

	bool isValidFileName(const std::string& fileName) {
		return std::find_first_of(fileName.begin(), fileName.end(), invalidChars.begin(), invalidChars.end()) == fileName.end();
	}
	std::string getUniqueFilename(const std::string& originalFilename) {
		try {
			std::string extension = originalFilename.substr(originalFilename.find_last_of('.'));
			std::string baseFilename = originalFilename.substr(0, originalFilename.find_last_of('.'));

			std::time_t now = std::time(nullptr);
			std::tm localTime;
			localtime_s(&localTime, &now);

			std::ostringstream oss;
			oss << baseFilename << "_"
				<< std::put_time(&localTime, "%Y_%m_%d__%H_%M_%S")
				<< extension;

			return oss.str();
		}
		catch (const std::exception& e) {
			printColoredMessage("Ошибка при генерации уникального имени файла: " + std::string(e.what()), CONSOLE_RED);
			return originalFilename; // возвращаемся к исходному имени в случае ошибки
		}
	}

	bool resolveDuplicate(
		const std::string& directoryPath,
		const std::string& binaryPath,
		const std::string& imagePath,
		const std::string& hashFilePath,
		fs::path& fileRelPath,
		std::string& fileHash,
		const cv::Mat& image
	) {
		try {
			bool flagExit = false;
			printColoredMessage("\nФайл с названием \"" + fileRelPath.string() + "\" уже есть в " + binaryPath, CONSOLE_DARK_YELLOW);
			do {
				printColoredMessage("\n1) Просмотреть сохраненный файл " + binaryPath + '\\' + fileRelPath.string(), CONSOLE_CYAN);
				printColoredMessage("2) Просмотреть файл " + imagePath, CONSOLE_YELLOW);
				printColoredMessage("3) Сохранить файл с другим названием.", CONSOLE_CYAN);
				printColoredMessage("4) Пропустить этот файл.\n", CONSOLE_YELLOW);

				SET_CONSOLE_MAGENTA;
				uchar x;
				std::cin >> x;
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				SET_CONSOLE_DEFAULT;

				switch (x) {
				case '1':
					printColoredMessage("\nПоиск " + fileRelPath.string() + " в " + binaryPath + ". . .", CONSOLE_BLUE);
					findAndDisplayImage(binaryPath, fileRelPath.string());
					break;
				case '2':
					displayImage(image, imagePath);
					break;
				case '3': {
					do {
						std::cout << "\n\n";
						printColoredMessage("1) Добавить к имени файла дату и время", CONSOLE_CYAN);
						printColoredMessage("2) Ввести новое название с клавиатуры\n", CONSOLE_YELLOW);

						SET_CONSOLE_MAGENTA;
						uchar x;
						std::cin >> x;
						std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						SET_CONSOLE_DEFAULT;

						switch (x)
						{
						case '1':
							fileRelPath = fs::path(getUniqueFilename(fileRelPath.string()));
							printColoredMessage("\nНовое имя файла: " + fileRelPath.string(), CONSOLE_YELLOW);
							fileHash = safe_hashing::sha256(RCC::encryptFilename(fileRelPath.wstring(), RCC_Shift));
							return false;
						case '2':
						{
							std::locale::global(std::locale("Russian_Russia.1251"));
							printColoredMessage("\nВведите новое название (без расширения): ", CONSOLE_BLUE, "");
							std::string newFilename;
							std::getline(std::cin, newFilename);

							while (true)
							{
								while (newFilename.empty() || newFilename == "" || !isValidFileName(newFilename))
								{
									printColoredMessage("\nИмя файла не должно содержать следующих символов: " + invalidChars + "\nПовторите ввод: ", CONSOLE_DARK_YELLOW, "");
									std::getline(std::cin, newFilename);

								}
								fileRelPath = fileRelPath.parent_path() / (newFilename + fileRelPath.extension().string());

								fileHash = safe_hashing::sha256(RCC::encryptFilename(fileRelPath.wstring(), RCC_Shift));
								if (isNameHashInFile(hashFilePath, fileHash)) {
									printColoredMessage("Файл " + fileRelPath.string() + " уже существует. Выберете другое имя", CONSOLE_YELLOW);
									newFilename.clear();
								}
								else {
									printColoredMessage("\nНовое имя файла: " + fileRelPath.string(), CONSOLE_YELLOW);
									return false;
								}
							}
						}
						default:
							printColoredMessage("Некорректный ввод: " + std::to_string(x) + ". Повторите попытку\n", CONSOLE_RED);
							break;
						}
					} while (true);

					break;
				}
				case '4':
					SET_CONSOLE_DEFAULT;
					std::cout << "\nФайл " + fileRelPath.string() + " остался в " + directoryPath + " и не был сохранен в " + binaryPath << std::endl;
					return true;
				default:
					printColoredMessage("Некорректный ввод. Повторите попытку\n", CONSOLE_RED);
				}
			} while (true);
		}
		catch (const std::exception& e) {
			printColoredMessage("Ошибка при разрешении дубликата: " + std::string(e.what()), CONSOLE_RED);
			return true; // true означает пропуск данного изображения
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при разрешении дубликата. Файл пропущен.", CONSOLE_RED);
			return true; // true означает пропуск данного изображения
		}
		return false;
	}

	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	std::string get_last_split_element(const std::string& str, char delimiter) {
		std::vector<std::string> tokens = split(str, delimiter);
		if (!tokens.empty()) {
			return tokens.back();
		}
		return "";
	}

	std::string ltrim(const std::string& s) {
		std::string result = s;
		result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {return !std::isspace(ch); }));
		return result;
	}

	std::string rtrim(const std::string& s) {
		std::string result = s;
		result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), result.end());
		return result;
	}

	std::string trim(const std::string& s) {
		return ltrim(rtrim(s));
	}

	fs::path trimPath(fs::path path) {
		std::vector < std::string> pathParts = split(path.string(), fs::path::preferred_separator);
		for (size_t i = 0; i < pathParts.size(); i++)
		{
			pathParts[i] = trim(pathParts[i]);
		}
		return make_path(pathParts);
	}

	fs::path addSuffixIfExists(const fs::path& directory_path, const fs::path& file_name) {
		fs::path full_path = directory_path / file_name;

		if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
			int counter = 1;
			fs::path new_path;
			do {
				new_path = directory_path / (full_path.stem().string() + " (" + std::to_string(counter) + ")" + full_path.extension().string());
				++counter;
			} while (fs::exists(new_path));
			return new_path;
		}
		return full_path;
	}

	void xorEncryptDecrypt(std::vector<uchar>& data, const std::string& key)
	{
		size_t keyLen = key.size();
		size_t dataLen = data.size() < DEFAULT_CHUNK_SIZE ? data.size() : DEFAULT_CHUNK_SIZE;
		for (size_t i = 0; i < dataLen; ++i)
		{
			data[i] ^= key[i % keyLen];
		}
	}
} //imghider