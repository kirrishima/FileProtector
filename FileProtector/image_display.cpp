#include "stdafx.h"
#include "opencv2/core/mat.hpp"

namespace imghider {
	bool displayImage(const cv::Mat& image, const std::string& displayName) noexcept
	{
		try
		{
			if (image.empty())
				throw;
			int width = 512 <= image.cols ? 512 : image.cols;
			int height = static_cast<int>(width * image.rows / static_cast<double>(image.cols));
			cv::Mat resizedImage;
			cv::resize(image, resizedImage, cv::Size(width, height));
			cv::imshow(displayName, resizedImage);
			cv::waitKey(0);
			cv::destroyAllWindows();
		}
		catch (const cv::Exception& e)
		{
			printColoredMessage("Ошибка OpenCV: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (const std::exception& e)
		{
			printColoredMessage("Общая Ошибка: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (...)
		{
			printColoredMessage("Неизвестная ошибка при выводе изображения", CONSOLE_RED);
			return false;
		}
		return true;
	}

	cv::Mat findImage(const std::string& binaryPath,
		const std::string& searchFilename,
		std::string* foundImageName,
		bool exactMatch,
		const std::string& outputDirectory) noexcept
	{
		try {
			// Локальный хелпер для единообразного сообщения об ошибке и возврата пустой матрицы
			auto make_error_mat = [&](const std::string& msg = std::string()) -> cv::Mat {
				if (!msg.empty()) printColoredMessage(msg, CONSOLE_RED);
				return cv::Mat();
				};

			std::ifstream file(binaryPath, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				return make_error_mat("Ошибка: не удалось открыть файл " + binaryPath);
			}

			static cv::Mat cachedImage;
			static std::string cachedImageName;

			if (exactMatch && !cachedImage.empty() && !cachedImageName.empty() && cachedImageName == searchFilename) {
				printColoredMessage("\nБыло использовано изображение \"" + cachedImageName + "\" найденное при прошлом поиске", CONSOLE_DARK_YELLOW);
				if (foundImageName != nullptr)
					*foundImageName = cachedImageName;
				return cachedImage;
			}

			// размер файла
			std::streampos tp = file.tellg();
			if (tp == static_cast<std::streampos>(-1)) {
				file.close();
				return make_error_mat("Не удалось определить размер файла: " + binaryPath);
			}
			size_t fileSize = file.tellg();
			file.close();

			size_t start = 0;
			while (start < fileSize) {
				// получаем метаданные (без чтения буфера изображения)
				auto metaOpt = loadImageMetadataFromBinary(binaryPath, start);
				if (!metaOpt.has_value()) {
					// Ошибка при чтении метаданных — логируем и завершаем
					printColoredMessage("Ошибка при чтении метаданных из \"" + binaryPath + "\".", CONSOLE_RED);
					return cv::Mat();
				}
				ImageMetadata meta = *metaOpt;

				// точное совпадение имени
				if (exactMatch && meta.name == searchFilename) {
					// загружаем полное изображение только сейчас
					auto imageTuple = loadImageFromBinary(binaryPath, start);
					// структурированное привязывание (c++17)
					cv::Mat image;
					std::string loadedName;
					size_t fileEndPos = start;
					bool success;
					std::tie(image, loadedName, fileEndPos, success) = imageTuple;

					if (!success) {
						printColoredMessage("Ошибка при загрузке изображения для полного совпадения.", CONSOLE_RED);
						return cv::Mat();
					}

					if (foundImageName != nullptr) *foundImageName = loadedName;
					cachedImageName = loadedName;
					cachedImage = image;
					return image;
				}

				// Нечёткий поиск (по подстроке) — сравнение имён в нижнем регистре
				// Нечёткий поиск (по подстроке) — сравнение по всей строке пути в нижнем регистре
				{
					std::wstring lowercaseImageName = string_to_wstring(meta.name);
					std::wstring lowercaseSearchFilename = string_to_wstring(searchFilename);

					std::transform(lowercaseImageName.begin(), lowercaseImageName.end(), lowercaseImageName.begin(), towlower);
					std::transform(lowercaseSearchFilename.begin(), lowercaseSearchFilename.end(), lowercaseSearchFilename.begin(), towlower);

					if (lowercaseImageName.find(lowercaseSearchFilename) != std::wstring::npos)
					{
						// нашли совпадение по подстроке — загружаем изображение и сохраняем результат,
						// восстанавливая исходную файловую структуру
						auto imageTuple = loadImageFromBinary(binaryPath, start);
						cv::Mat image;
						std::string loadedName;
						size_t fileEndPos = start;
						bool success;
						std::tie(image, loadedName, fileEndPos, success) = imageTuple;

						if (!success) {
							printColoredMessage("Ошибка при загрузке изображения (для сохранения результатов поиска).", CONSOLE_RED);
							// продолжаем обработку следующих записей
						}
						else {
							try {
								// Оригинальный путь (в виде строки) — может быть относительным или абсолютным
								fs::path originalPath = fs::path(loadedName);

								// Получаем "относительную" часть пути:
								// - если originalPath абсолютный, relativePath будет без root_name/root_directory (т.е. без диска)
								// - если относительный — используем его как есть
								fs::path relativePath = originalPath.is_absolute() ? originalPath.relative_path() : originalPath;

								// Базовая папка результатов для этого поиска
								fs::path resultsBase = fs::path(outputDirectory) / fs::path("search results for " + searchFilename);

								// Целевая директория — повторяем оригинальную структуру (без имени файла)
								fs::path targetDir = resultsBase / relativePath.parent_path();

								// Создаем директории рекурсивно (без исключений)
								std::error_code ec;
								if (!targetDir.empty()) {
									fs::create_directories(targetDir, ec);
								}
								else {
									// если parent_path() пуст — используем resultsBase
									fs::create_directories(resultsBase, ec);
									targetDir = resultsBase;
								}

								if (ec) {
									printColoredMessage("Не удалось создать директорию для сохранения: " + targetDir.string() + " : " + ec.message(), CONSOLE_RED);
								}
								else {
									// Сохраняем с оригинальным именем файла в восстановленной структуре
									std::string filenameOnly = fs::path(loadedName).filename().string();
									saveImage(image, filenameOnly, targetDir);
								}
							}
							catch (const std::exception& e) {
								printColoredMessage("Ошибка при сохранении найденного изображения: " + std::string(e.what()), CONSOLE_RED);
							}
							catch (...) {
								printColoredMessage("Неизвестная ошибка при сохранении найденного изображения.", CONSOLE_RED);
							}
						}
					}
				}


				// Продвигаем start на позицию следующей записи (fileEndPos из метаданных)
				start = meta.fileEndPos;
			}

			// ничего не найдено
			return cv::Mat();
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("Ошибка при работе с файлом: " + std::string(e.what()), CONSOLE_RED);
			return cv::Mat();
		}
		catch (const std::exception& e) {
			printColoredMessage("Общая ошибка: " + std::string(e.what()), CONSOLE_RED);
			return cv::Mat();
		}
		catch (...) {
			printColoredMessage("Неизвестная ошибка.", CONSOLE_RED);
			return cv::Mat();
		}
	}

	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename) noexcept {
		checkAndCreatePaths({ {binaryPath} });

		std::string imageName;
		const cv::Mat image = findImage(binaryPath, searchFilename, &imageName);
		if (image.empty() || imageName.empty())
		{
			return false;
		}
		return displayImage(image, (fs::path(binaryPath) / imageName).string());
	}
}