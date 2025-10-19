#include "stdafx.h"

namespace imghider {
	bool saveImageToBinary(const std::string& binaryPath, const std::string& imageName, const cv::Mat& image) {
		try {
			std::ofstream binaryFile(binaryPath, std::ios::out | std::ios::binary | std::ios::app);

			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return false;
			}

			size_t imageNameLength = imageName.size();
			binaryFile.write(reinterpret_cast<const char*>(&imageNameLength), sizeof(size_t));
			binaryFile.write(imageName.c_str(), imageNameLength);

			int rows = image.rows;
			int cols = image.cols;
			int type = image.type();

			binaryFile.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
			binaryFile.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
			binaryFile.write(reinterpret_cast<const char*>(&type), sizeof(type));

			// Изображение сжимается в исходном формате для экономии памяти
			std::vector<uchar> buffer;
			std::string extension = imageName.substr(imageName.find_last_of('.'));
			cv::imencode(extension, image, buffer);

			// Кодируется часть изображения
			xorEncryptDecrypt(buffer, ENCRYPTING_KEY);
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

						if (!isImageFile(imagePath))
						{
							printColoredMessage("\nФайл " + imagePath + " пропущен, так как его расширение не поддерживается.", CONSOLE_DARK_YELLOW);
							continue;
						}
						fs::path imgRelPath = fs::relative(entry.path(), fs::path(directoryPath));

						cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
						if (image.empty()) {
							printColoredMessage("Ошибка: не удалось загрузить изображение " + imagePath, CONSOLE_RED);
							continue;
						}

						std::string fileHash = safe_hashing::sha256(RCC::encryptFilename((imgRelPath).wstring(), RCC_Shift));
						if (isNameHashInFile(hashFilePath, fileHash)) {
							if (resolveDuplicate(directoryPath, binaryPath, imagePath, hashFilePath, imgRelPath, fileHash, image)) {
								continue;
							}
						}

						addNameHashToFile(hashFilePath, fileHash);
						if (saveImageToBinary(binaryPath, RCC::encryptFilename(imgRelPath.wstring(), RCC_Shift), image)) {
							printColoredMessage("\nФайл " + imgRelPath.string() + " успешно добавлен в " + binaryPath, CONSOLE_GREEN, "");
							if (fs::remove(imagePath)) {
								printColoredMessage(" и удален из " + directoryPath + '.', CONSOLE_GREEN);
							}
							else {
								printColoredMessage("\nОшибка: не удалось удалить файл " + imgRelPath.string(), CONSOLE_RED);
							}
						}
						else {
							printColoredMessage("\nНе удалось записать файл " + imgRelPath.string() + " в " + binaryPath, CONSOLE_RED);
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

	bool saveImage(const cv::Mat image, const fs::path& imageName, const fs::path& outputDirectory) noexcept {
		try {

			fs::path outputPath = outputDirectory / imageName;
			fs::path fileParentPath = outputPath.parent_path();

			outputPath = trimPath(outputPath);
			fileParentPath = trimPath(fileParentPath);

			if (!fs::exists(fileParentPath)) {
				try {
					fs::create_directories(fileParentPath);
				}
				catch (const std::exception& e) {
					printColoredMessage("Не удалось создать директорию " + fileParentPath.string() + " для изображения: " + std::string(e.what()), CONSOLE_RED);
					if (!fs::exists(outputDirectory)) {
						try {
							fs::create_directories(outputDirectory);
						}
						catch (const std::exception& e) {
							printColoredMessage("Не удалось создать " + outputDirectory.string() + ": " + e.what(), CONSOLE_RED);
							return false;
						}
						catch (...) {
							printColoredMessage("Неизвестная ошибка при создании + outputDirectory", CONSOLE_RED);
							return false;
						}
					}
					if (!fs::exists(outputDirectory / imageName.filename()))
					{
						outputPath = outputDirectory / imageName.filename();
					}
					else {
						do {
							outputPath = outputDirectory / fs::path(getUniqueFilename(imageName.string())).filename();
						} while (fs::exists(outputPath));
					}
				}
			}
			outputPath = addSuffixIfExists(outputPath.parent_path(), outputPath.filename());
			if (!cv::imwrite(outputPath.string(), image)) {
				printColoredMessage("Ошибка: не удалось восстановить изображение " + outputPath.string(), CONSOLE_RED);
			}
			else {
				printColoredMessage("\nВосстановлен файл ", CONSOLE_GREEN, "");
				printColoredMessage(outputPath.string(), CONSOLE_CYAN);
			}
			return true;
		}
		catch (const std::exception& e) {
			printColoredMessage("Ошибка при обработке изображения: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка при обработке изображения.", CONSOLE_RED);
			return false;
		}

	}

	std::tuple<cv::Mat, std::string, size_t, bool> loadImageFromBinary(const std::string& binaryPath, const size_t start) {
		try {
			std::ifstream binaryFile(binaryPath, std::ios::in | std::ios::binary);

			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return std::make_tuple(cv::Mat(), std::string(), -1, false);
			}

			binaryFile.seekg(start);

			size_t imageNameLength;
			binaryFile.read(reinterpret_cast<char*>(&imageNameLength), sizeof(size_t));

			std::vector<char> imageNameBuffer(imageNameLength);
			binaryFile.read(imageNameBuffer.data(), imageNameLength);

			std::string imageName(imageNameBuffer.data(), imageNameLength);
			imageName = RCC::encryptFilename(string_to_wstring(imageName), -RCC_Shift);

			int rows, cols, type;
			binaryFile.read(reinterpret_cast<char*>(&rows), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&cols), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&type), sizeof(int));

			int imageDataBufferSize;
			binaryFile.read(reinterpret_cast<char*>(&imageDataBufferSize), sizeof(int));
			std::vector<uchar> buffer(imageDataBufferSize);
			binaryFile.read(reinterpret_cast<char*>(buffer.data()), imageDataBufferSize);

			// Декодируем 
			xorEncryptDecrypt(buffer, ENCRYPTING_KEY);

			size_t fileEndPos = binaryFile.tellg();
			binaryFile.close();

			cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
			if (image.empty() || image.rows != rows || image.cols != cols || image.type() != type) {
				return std::make_tuple(cv::Mat(), std::string(), fileEndPos, false);
			}

			return std::make_tuple(image, imageName, fileEndPos, true);
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

	void loadAndSaveImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory) {
		try {
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
				auto imageData = loadImageFromBinary(binaryPath, start);
				const cv::Mat& image = std::get<0>(imageData);
				const std::string imageName = std::get<1>(imageData);
				start = std::get<2>(imageData);
				const bool success = std::get<3>(imageData);

				if (!success || image.empty() || imageName.empty()) {
					printColoredMessage("Ошибка при чтении изображения. Проверьте наличие файла " + binaryPath + ". Возможно, файл поврежден.", CONSOLE_RED);
					continue;
				}

				saveImage(image, fs::path(imageName), fs::path(outputDirectory));
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

	bool checkAndCreatePaths(const std::vector<std::string>& paths, bool verbose) {
		bool createdAny = false;
		for (const auto& path : paths) {
			fs::path fsPath(path);
			try {
				if (fsPath.has_extension()) {
					fs::path parentPath = fsPath.parent_path();
					if (!parentPath.empty() && !fs::exists(parentPath)) {
						fs::create_directories(parentPath);
					}
					if (!fs::exists(fsPath)) {
						std::ofstream file(fsPath);
						if (file) {
							createdAny = true;
							if (verbose) {
								printColoredMessage("Создан файл: " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (verbose) {
								printColoredMessage("Ошибка создания файла: " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (verbose) {
							printColoredMessage("Файл уже существует: " + fsPath.string(), CONSOLE_YELLOW);
						}
					}
				}
				else {
					if (!fs::exists(fsPath)) {
						if (fs::create_directories(fsPath)) {
							createdAny = true;
							if (verbose) {
								printColoredMessage("Создана директория " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (verbose) {
								printColoredMessage("Не удалось создать директорию " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (verbose) {
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

	fs::path make_path(const std::vector<std::string>& parts) {
		fs::path p;
		for (const auto& part : parts) {
			p /= part;
		}
		return p;
	}

	bool isImageFile(const fs::path& file_name) {

		fs::path file_path(file_name);
		std::string extension = file_path.extension().string();

		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

		return IMAGE_EXTENSIONS.find(extension) != IMAGE_EXTENSIONS.end();
	}
}