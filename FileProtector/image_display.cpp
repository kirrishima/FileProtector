#include "stdafx.h"
#include "imghider.h"

namespace imghider {
	bool displayImage(const cv::Mat& image, const std::string& displayName)
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
			printColoredMessage("Неизвестная ошибка", CONSOLE_RED);
			return false;
		}
		return true;
	}
	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename) {
		try {
			checkAndCreatePaths({ {binaryPath} });

			std::ifstream file(binaryPath, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				printColoredMessage("Ошибка: не удалось открыть файл " + binaryPath, CONSOLE_RED);
				return false;
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
						printColoredMessage("Ошибка при чтении изображения из \"" + binaryPath + "\".", CONSOLE_RED);
						return false;
					}
					if (imageName == searchFilename) {
						return displayImage(image, createPath(binaryPath, imageName));
					}
					start = newStart;
				}
				catch (const std::exception& e) {
					printColoredMessage("Ошибка при загрузке изображения: " + std::string(e.what()), CONSOLE_RED);
					return false;
				}
				catch (...) {
					printColoredMessage("Неизвестная Ошибка при загрузке изображения", CONSOLE_RED);
					return false;
				}
			}
		}
		catch (const std::ifstream::failure& e) {
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

		return false;
	}
}