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

			std::vector<uchar> buffer;
			std::string extension = imageName.substr(imageName.find_last_of('.'));
			cv::imencode(extension, image, buffer);

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

	std::optional<ImageMetadata> loadImageMetadataFromBinary(const std::string& binaryPath, const size_t start) noexcept
	{
		try {
			std::ifstream binaryFile(binaryPath, std::ios::in | std::ios::binary);
			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return std::nullopt;
			}

			auto make_error_and_close_file = [&](const char* msg = nullptr) -> std::optional<ImageMetadata> {
				if (msg) printColoredMessage(std::string("Ошибка (метаданные): ") + msg, CONSOLE_RED);
				binaryFile.close();
				return std::nullopt;
				};

			binaryFile.seekg(0, std::ios::end);
			std::streampos t = binaryFile.tellg();
			if (t == static_cast<std::streampos>(-1)) return make_error_and_close_file("Не удалось определить размер файла");
			size_t fileSize = static_cast<size_t>(t);

			if (start >= fileSize) {
				return make_error_and_close_file("start >= fileSize");
			}

			binaryFile.seekg(static_cast<std::streamoff>(start), std::ios::beg);
			if (!binaryFile.good()) return make_error_and_close_file("seekg(start) неудачен");

			size_t imageNameLength = 0;
			binaryFile.read(reinterpret_cast<char*>(&imageNameLength), sizeof(size_t));
			if (!binaryFile.good()) return make_error_and_close_file("Не удалось прочитать длину имени");

			const size_t MAX_NAME_LEN = 1 << 20;
			if (imageNameLength == 0 || imageNameLength > MAX_NAME_LEN) {
				return make_error_and_close_file("Неверная длина имени");
			}

			size_t minimalNeeded = sizeof(size_t) + imageNameLength + 3 * sizeof(int) + sizeof(int);
			if (start + minimalNeeded > fileSize) {
				return make_error_and_close_file("Недостаточно байт для чтения заголовка");
			}

			std::vector<char> imageNameBuffer(imageNameLength);
			binaryFile.read(imageNameBuffer.data(), static_cast<std::streamsize>(imageNameLength));
			if (!binaryFile.good()) return make_error_and_close_file("Не удалось прочитать имя");

			std::string rawImageName(imageNameBuffer.data(), imageNameLength);
			std::string imageName = RCC::encryptFilename(string_to_wstring(rawImageName), -RCC_Shift);

			int rows = 0, cols = 0, type = 0;
			binaryFile.read(reinterpret_cast<char*>(&rows), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&cols), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&type), sizeof(int));
			if (!binaryFile.good()) return make_error_and_close_file("Не удалось прочитать rows/cols/type");

			int imageDataBufferSize = 0;
			binaryFile.read(reinterpret_cast<char*>(&imageDataBufferSize), sizeof(int));
			if (!binaryFile.good() || imageDataBufferSize < 0) return make_error_and_close_file("Неверный imageDataBufferSize");

			std::streampos curPos = binaryFile.tellg();
			if (curPos == static_cast<std::streampos>(-1)) return make_error_and_close_file("tellg() не удался");
			size_t bufferStartPos = static_cast<size_t>(curPos);

			if (bufferStartPos + static_cast<size_t>(imageDataBufferSize) > fileSize) {
				return make_error_and_close_file("Буфер изображения выходит за пределы файла");
			}

			size_t fileEndPos = bufferStartPos + static_cast<size_t>(imageDataBufferSize);

			ImageMetadata meta;
			meta.name = std::move(imageName);
			meta.bufferStartPos = bufferStartPos;
			meta.fileEndPos = fileEndPos;
			meta.rows = rows;
			meta.cols = cols;
			meta.type = type;
			meta.bufferSize = imageDataBufferSize;

			binaryFile.close();
			return meta;
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("Ошибка при работе с файлом (метаданные): " + std::string(e.what()), CONSOLE_RED);
			return std::nullopt;
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка (метаданные): " + std::string(e.what()), CONSOLE_RED);
			return std::nullopt;
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка (метаданные).", CONSOLE_RED);
			return std::nullopt;
		}
	}

	/**
	 * @brief Загружает изображение из бинарного файла (использует loadImageMetadataFromBinary для парсинга метаданных).
	 *
	 * @param binaryPath: Путь к бинарному файлу.
	 * @param start: Начальная позиция в бинарном файле, с которой начинается запись изображения.
	 * @return std::tuple<cv::Mat, std::string, size_t, bool>
	 *         Кортеж содержащий:
	 *           - cv::Mat изображения (пустой, если не удалось загрузить),
	 *           - название изображения (после преобразования),
	 *           - позицию в файле после чтения (т.е. fileEndPos),
	 *           - флаг успеха.
	 */
	std::tuple<cv::Mat, std::string, size_t, bool> loadImageFromBinary(const std::string& binaryPath, const size_t start) {
		try {
			// локальный хелпер для единообразного возврата ошибки
			auto make_error = [&](size_t retPos) {
				return std::make_tuple(cv::Mat(), std::string(), retPos, false);
				};

			// Получаем метаданные (читаем быстро, без загрузки буфера)
			auto metaOpt = loadImageMetadataFromBinary(binaryPath, start);
			if (!metaOpt.has_value()) {
				return make_error(start);
			}
			ImageMetadata meta = *metaOpt;

			// Открываем файл и читаем буфер изображения (начиная с bufferStartPos)
			std::ifstream binaryFile(binaryPath, std::ios::in | std::ios::binary);
			if (!binaryFile.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return make_error(start);
			}

			binaryFile.seekg(static_cast<std::streamoff>(meta.bufferStartPos), std::ios::beg);
			if (!binaryFile.good()) {
				binaryFile.close();
				return make_error(start);
			}

			std::vector<uchar> buffer(static_cast<size_t>(meta.bufferSize));
			if (meta.bufferSize > 0) {
				binaryFile.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(meta.bufferSize));
				if (!binaryFile.good()) {
					binaryFile.close();
					return make_error(start);
				}
			}

			size_t fileEndPos = meta.fileEndPos;
			binaryFile.close();

			// Декодирование / расшифровка
			xorEncryptDecrypt(buffer, ENCRYPTING_KEY);

			cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
			if (image.empty() || image.rows != meta.rows || image.cols != meta.cols || image.type() != meta.type) {
				return std::make_tuple(cv::Mat(), meta.name, fileEndPos, false);
			}

			return std::make_tuple(image, meta.name, fileEndPos, true);
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