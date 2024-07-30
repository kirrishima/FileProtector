#include "stdafx.h"
#include "imghider.h"
#include <filesystem>
#include "VideoEncryptor.h"
#include "hashing.h"

namespace imghider {

	bool saveImage(const std::string& binaryPath, const std::string& imageName, const cv::Mat& image) {
		try {
			std::ofstream binaryFile(binaryPath, std::ios::out | std::ios::binary | std::ios::app);

			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return false;
			}

			// Записываем длину названия изображения и само название
			size_t imageNameLength = imageName.size();
			binaryFile.write(reinterpret_cast<const char*>(&imageNameLength), sizeof(size_t));
			binaryFile.write(imageName.c_str(), imageNameLength);

			// Записываем метаданные
			int rows = image.rows;
			int cols = image.cols;
			int type = image.type();

			binaryFile.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
			binaryFile.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
			binaryFile.write(reinterpret_cast<const char*>(&type), sizeof(type));

			// Сжимаем изображение в формате PNG и записываем
			std::vector<uchar> buffer;
			std::string extension = imageName.substr(imageName.find_last_of('.'));
			cv::imencode(extension, image, buffer);
			int bufferSize = buffer.size();
			binaryFile.write(reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize));
			binaryFile.write(reinterpret_cast<const char*>(buffer.data()), bufferSize);

			binaryFile.close();
			return true;
		}
		catch (const std::ofstream::failure& e) {
			printColoredMessage("Ошибка при работе с файлом: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка.", CONSOLE_RED);
			return false;
		}
	}

	void saveImagesToBinary(const std::string& directoryPath, const std::string& binaryPath, const std::string& hashFilePath) {
		try {
			checkAndCreatePaths({ {directoryPath, binaryPath, hashFilePath} });

			for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
				if (entry.is_regular_file()) {
					try {
						std::string imagePath = entry.path().string();
						std::string imageFilename = entry.path().filename().string();
						std::string newImageFilename = imageFilename;

						fs::path imgRelPath = fs::relative(entry.path(), fs::path(directoryPath));
						/*imgRelPath = imgRelPath.parent_path().empty() ? fs::path("") : imgRelPath;*/

						cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

						if (image.empty()) {
							printColoredMessage("Ошибка: не удалось загрузить изображение " + imagePath, CONSOLE_RED);
							continue;
						}

						std::string fileHash = safe_hashing::sha256(RCC::encryptFilename((imgRelPath / imageFilename).string(), RCC_Shift));
						if (isNameHashInFile(hashFilePath, fileHash)) {
							if (resolveDuplicate(directoryPath, binaryPath, imagePath, imageFilename, (imgRelPath).string(), newImageFilename, fileHash, image)) {
								continue;
							}
						}

						addNameHashToFile(hashFilePath, fileHash);
						if (saveImage(binaryPath, RCC::encryptFilename((imgRelPath.parent_path() / newImageFilename).string(), RCC_Shift), image)) {
							printColoredMessage("Файл " + imagePath + " успешно добавлен в " + binaryPath, CONSOLE_GREEN, "");
							if (fs::remove(imagePath)) {
								printColoredMessage(" и удален из " + directoryPath + '.', CONSOLE_GREEN);
							}
							else {
								printColoredMessage("\nОшибка: не удалось удалить файл " + imagePath, CONSOLE_RED);
							}
						}
						else {
							printColoredMessage("Не удалось записать файл " + imagePath + " в " + binaryPath, CONSOLE_RED);
						}
					}
					catch (const std::exception& e) {
						printColoredMessage("Ошибка при обработке файла " + entry.path().string() + ": " + std::string(e.what()), CONSOLE_RED);
					}
					catch (...) {
						printColoredMessage("Неизвестная ошибка при обработке файла " + entry.path().string(), CONSOLE_RED);
					}
				}
			}
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка.", CONSOLE_RED);
		}
	}

	std::tuple<cv::Mat, std::string, size_t, bool> loadImage(const std::string& binaryPath, const size_t start) {
		try {
			std::ifstream binaryFile(binaryPath, std::ios::in | std::ios::binary);

			// Проверяем, удалось ли открыть файл
			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return std::make_tuple(cv::Mat(), std::string(), -1, false);
			}

			// Перемещаем указатель чтения на заданную начальную позицию
			binaryFile.seekg(start);

			// Читаем длину имени файла
			size_t imageNameLength;
			binaryFile.read(reinterpret_cast<char*>(&imageNameLength), sizeof(size_t));

			// Читаем имя файла
			std::vector<char> imageNameBuffer(imageNameLength);
			binaryFile.read(imageNameBuffer.data(), imageNameLength);

			// Декодируем и расшифровываем имя файла
			std::string imageName(imageNameBuffer.data(), imageNameLength);
			imageName = RCC::encryptFilename(imageName, -RCC_Shift);

			// Читаем метаданные изображения
			int rows, cols, type;
			binaryFile.read(reinterpret_cast<char*>(&rows), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&cols), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&type), sizeof(int));

			// Читаем размер буфера и данные изображения
			int imageDataBufferSize;
			binaryFile.read(reinterpret_cast<char*>(&imageDataBufferSize), sizeof(int));
			std::vector<uchar> buffer(imageDataBufferSize);
			binaryFile.read(reinterpret_cast<char*>(buffer.data()), imageDataBufferSize);

			// Получаем текущую позицию указателя в файле
			size_t binaryFileTellg = binaryFile.tellg();
			binaryFile.close();

			// Декодируем изображение из буфера
			cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
			if (image.empty() || image.rows != rows || image.cols != cols || image.type() != type) {
				return std::make_tuple(cv::Mat(), std::string(), binaryFileTellg, false);
			}

			// Возвращаем загруженное изображение, имя файла, текущую позицию в файле и флаг успеха
			return std::make_tuple(image, imageName, binaryFileTellg, true);
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("Ошибка при работе с файлом: " + std::string(e.what()), CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка.", CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
	}

	void loadImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory) {
		try {
			// Проверяем и создаем пути, если необходимо
			checkAndCreatePaths({ {outputDirectory, binaryPath} });

			std::ifstream file(binaryPath, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return;
			}

			size_t fileSize = file.tellg();
			file.close();
			size_t start = 0;

			while (start < fileSize) {
				try {
					auto imageData = loadImage(binaryPath, start);
					const cv::Mat& image = std::get<0>(imageData);
					const std::string& imageName = std::get<1>(imageData);
					size_t newStart = std::get<2>(imageData);
					bool success = std::get<3>(imageData);

					if (!success) {
						printColoredMessage("Ошибка при чтении изображения. Проверьте наличие файла " + binaryPath + " и повторите попытку.", CONSOLE_RED);
						return;
					}


					fs::path outputPath = fs::path(outputDirectory) / fs::path(imageName);
					fs::path filePathExceptFilename = outputPath.parent_path();

					if (!fs::exists(filePathExceptFilename)) {
						try {
							fs::create_directories(filePathExceptFilename);
						}
						catch (const std::exception& e) {
							printColoredMessage("Не удалось создать директорию " + filePathExceptFilename.string() + " для изображения: " + std::string(e.what()), CONSOLE_RED);
							// Попытка записи в базовую директорию
							if (!fs::exists(outputDirectory)) {
								try {
									fs::create_directories(outputDirectory);
								}
								catch (const std::exception& e) {
									printColoredMessage("Не удалось создать " + outputDirectory + ": " + e.what(), CONSOLE_RED);
									return; // Прерываем выполнение функции, если не удалось создать директорию
								}
								catch (...) {
									printColoredMessage("Неизвестная ошибка при создании + outputDirectory", CONSOLE_RED);
									return;
								}
							}
							if (!fs::exists(fs::path(outputDirectory) / fs::path(imageName).filename()))
							{
								outputPath = (fs::path(outputDirectory) / fs::path(imageName).filename()).string();
							}
							else {
								do {
									outputPath = (fs::path(outputDirectory) / fs::path(getUniqueFilename(imageName)).filename()).string();
								} while (fs::exists(outputPath));
							}
						}
					}
					if (!cv::imwrite(outputPath.string(), image)) {
						printColoredMessage("Ошибка: не удалось восстановить изображение " + outputPath.string(), CONSOLE_RED);
					}
					else {
						printColoredMessage("Восстановлен файл " + outputPath.string(), CONSOLE_GREEN);
						start = newStart;
					}
				}
				catch (const std::exception& e) {
					printColoredMessage("Ошибка при обработке изображения: " + std::string(e.what()), CONSOLE_RED);
				}
				catch (...) {
					printColoredMessage("Неизвестная ошибка при обработке изображения.", CONSOLE_RED);
				}
			}
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка.", CONSOLE_RED);
		}
	}

	void deleteEntry(const std::filesystem::directory_entry& entry) {
		const std::string filePath = entry.path().string();
		try {
			if (entry.is_regular_file()) {
				if (std::filesystem::remove(filePath)) {
					printColoredMessage("Файл " + filePath + " успешно удален.", CONSOLE_GREEN);
				}
				else {
					printColoredMessage("Ошибка: не удалось удалить файл " + filePath, CONSOLE_RED);
				}
			}
			else if (entry.is_directory()) {
				if (std::filesystem::remove_all(filePath)) {
					printColoredMessage("Папка " + filePath + " успешно удалена.", CONSOLE_GREEN);
				}
				else {
					printColoredMessage("Ошибка: не удалось удалить папку " + filePath, CONSOLE_RED);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			printColoredMessage("Ошибка файловой системы при удалении " + filePath + ": " + e.what(), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка при удалении " + filePath + ": " + e.what(), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при удалении " + filePath, CONSOLE_RED);
		}
	}

	void clearDirectory(const std::string& directoryPath, bool deleteSubfolders, bool clearSubfolders) {
		try {
			for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
				try {
					if (entry.is_regular_file()) {
						deleteEntry(entry);
					}
					else if (entry.is_directory() && deleteSubfolders) {
						deleteEntry(entry);
					}
					else if (entry.is_directory() && clearSubfolders) {
						clearDirectory(entry.path().string(), deleteSubfolders, clearSubfolders);
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					printColoredMessage("Ошибка файловой системы при обработке " + entry.path().string() + ": " + e.what(), CONSOLE_RED);
				}
				catch (const std::exception& e) {
					printColoredMessage("Общая ошибка при обработке " + entry.path().string() + ": " + e.what(), CONSOLE_RED);
				}
				catch (...) {
					printColoredMessage("Неизвестная ошибка при обработке " + entry.path().string(), CONSOLE_RED);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			printColoredMessage("Ошибка файловой системы при итерации по " + directoryPath + ": " + e.what(), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка при итерации по " + directoryPath + ": " + e.what(), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при итерации по " + directoryPath, CONSOLE_RED);
		}
	}

	bool checkAndCreatePaths(const Params& params) {
		bool createdAny = false;
		for (const auto& path : params.paths) {
			fs::path fsPath(path);

			try {
				// Проверяем, является ли путь файлом или директорией
				if (fsPath.has_extension()) {
					// Это файл, создаем все родительские директории
					fs::path parentPath = fsPath.parent_path();
					if (!parentPath.empty() && !fs::exists(parentPath)) {
						fs::create_directories(parentPath);
					}
					// Создаем файл, если он не существует
					if (!fs::exists(fsPath)) {
						std::ofstream file(fsPath);
						if (file) {
							createdAny = true;
							if (params.verbose) {
								printColoredMessage("Создан файл: " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (params.verbose) {
								printColoredMessage("Ошибка создания файла: " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (params.verbose) {
							printColoredMessage("Файл уже существует: " + fsPath.string(), CONSOLE_YELLOW);
						}
					}
				}
				else {
					// Это директория, создаем все необходимые директории
					if (!fs::exists(fsPath)) {
						if (fs::create_directories(fsPath)) {
							createdAny = true;
							if (params.verbose) {
								printColoredMessage("Создана директория " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (params.verbose) {
								printColoredMessage("Не удалось создать директорию " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (params.verbose) {
							printColoredMessage("Директория " + fsPath.string() + " уже существует", CONSOLE_YELLOW);
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				printColoredMessage("Ошибка файловой системы: " + std::string(e.what()), CONSOLE_RED);
			}
			catch (const std::exception& e) {
				printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
			}
			catch (...) {
				printColoredMessage("Неизвестная ошибка при обработке пути: " + fsPath.string(), CONSOLE_RED);
			}
		}
		return createdAny;
	}

	//template<typename... Paths>
	//std::string createPath(Paths... paths) {
	//	std::filesystem::path combinedPath;
	//	((combinedPath /= paths), ...);
	//	return combinedPath.string();
	//}
	//template std::string createPath<std::string>(std::string, std::string);
	//template std::string createPath<std::string, std::string>(std::string, std::string);
	//template std::string createPath<std::string, std::string, std::string>(std::string, std::string, std::string);
}