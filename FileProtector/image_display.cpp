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
			printColoredMessage("������ OpenCV: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (const std::exception& e)
		{
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (...)
		{
			printColoredMessage("����������� ������", CONSOLE_RED);
			return false;
		}
		return true;
	}
	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename) {
		try {
			checkAndCreatePaths({ {binaryPath} });

			std::ifstream file(binaryPath, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				printColoredMessage("������: �� ������� ������� ���� " + binaryPath, CONSOLE_RED);
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
						printColoredMessage("������ ��� ������ ����������� �� \"" + binaryPath + "\".", CONSOLE_RED);
						return false;
					}
					if (imageName == searchFilename) {
						return displayImage(image, createPath(binaryPath, imageName));
					}
					start = newStart;
				}
				catch (const std::exception& e) {
					printColoredMessage("������ ��� �������� �����������: " + std::string(e.what()), CONSOLE_RED);
					return false;
				}
				catch (...) {
					printColoredMessage("����������� ������ ��� �������� �����������", CONSOLE_RED);
					return false;
				}
			}
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("������ ��� ������ � ������: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
			return false;
		}
		catch (...) {
			printColoredMessage("����������� ������.", CONSOLE_RED);
			return false;
		}

		return false;
	}
}