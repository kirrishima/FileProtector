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
				printColoredMessage("������: �� ������� ������� ���� " + binaryPath, CONSOLE_RED);
				return false;
			}

			// ���������� ����� �������� ����������� � ���� ��������
			size_t imageNameLength = imageName.size();
			binaryFile.write(reinterpret_cast<const char*>(&imageNameLength), sizeof(size_t));
			binaryFile.write(imageName.c_str(), imageNameLength);

			// ���������� ����������
			int rows = image.rows;
			int cols = image.cols;
			int type = image.type();

			binaryFile.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
			binaryFile.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
			binaryFile.write(reinterpret_cast<const char*>(&type), sizeof(type));

			// ������� ����������� � ������� PNG � ����������
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
							printColoredMessage("������: �� ������� ��������� ����������� " + imagePath, CONSOLE_RED);
							continue;
						}

						std::string fileHash = safe_hashing::sha256(RCC::encryptFilename((imgRelPath).wstring(), RCC_Shift));
						if (isNameHashInFile(hashFilePath, fileHash)) {
							if (resolveDuplicate(directoryPath, binaryPath, imagePath, imageFilename, (imgRelPath).string(), newImageFilename, fileHash, image)) {
								continue;
							}
						}

						addNameHashToFile(hashFilePath, fileHash);
						if (saveImage(binaryPath, RCC::encryptFilename((imgRelPath.parent_path() / newImageFilename).wstring(), RCC_Shift), image)) {
							printColoredMessage("���� " + imagePath + " ������� �������� � " + binaryPath, CONSOLE_GREEN, "");
							if (fs::remove(imagePath)) {
								printColoredMessage(" � ������ �� " + directoryPath + '.', CONSOLE_GREEN);
							}
							else {
								printColoredMessage("\n������: �� ������� ������� ���� " + imagePath, CONSOLE_RED);
							}
						}
						else {
							printColoredMessage("�� ������� �������� ���� " + imagePath + " � " + binaryPath, CONSOLE_RED);
						}
					}
					catch (const std::exception& e) {
						printColoredMessage("������ ��� ��������� ����� " + entry.path().string() + ": " + std::string(e.what()), CONSOLE_RED);
					}
					catch (...) {
						printColoredMessage("����������� ������ ��� ��������� ����� " + entry.path().string(), CONSOLE_RED);
					}
				}
			}
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������.", CONSOLE_RED);
		}
	}

	std::tuple<cv::Mat, std::string, size_t, bool> loadImage(const std::string& binaryPath, const size_t start) {
		try {
			std::ifstream binaryFile(binaryPath, std::ios::in | std::ios::binary);

			// ���������, ������� �� ������� ����
			if (!binaryFile.is_open()) {
				printColoredMessage("������: �� ������� ������� ���� " + binaryPath, CONSOLE_RED);
				return std::make_tuple(cv::Mat(), std::string(), -1, false);
			}

			// ���������� ��������� ������ �� �������� ��������� �������
			binaryFile.seekg(start);

			// ������ ����� ����� �����
			size_t imageNameLength;
			binaryFile.read(reinterpret_cast<char*>(&imageNameLength), sizeof(size_t));

			// ������ ��� �����
			std::vector<char> imageNameBuffer(imageNameLength);
			binaryFile.read(imageNameBuffer.data(), imageNameLength);

			// ���������� � �������������� ��� �����
			std::string imageName(imageNameBuffer.data(), imageNameLength);
			imageName = RCC::encryptFilename(string_to_wstring(imageName), -RCC_Shift);

			// ������ ���������� �����������
			int rows, cols, type;
			binaryFile.read(reinterpret_cast<char*>(&rows), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&cols), sizeof(int));
			binaryFile.read(reinterpret_cast<char*>(&type), sizeof(int));

			// ������ ������ ������ � ������ �����������
			int imageDataBufferSize;
			binaryFile.read(reinterpret_cast<char*>(&imageDataBufferSize), sizeof(int));
			std::vector<uchar> buffer(imageDataBufferSize);
			binaryFile.read(reinterpret_cast<char*>(buffer.data()), imageDataBufferSize);

			// �������� ������� ������� ��������� � �����
			size_t binaryFileTellg = binaryFile.tellg();
			binaryFile.close();

			// ���������� ����������� �� ������
			cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
			if (image.empty() || image.rows != rows || image.cols != cols || image.type() != type) {
				return std::make_tuple(cv::Mat(), std::string(), binaryFileTellg, false);
			}

			// ���������� ����������� �����������, ��� �����, ������� ������� � ����� � ���� ������
			return std::make_tuple(image, imageName, binaryFileTellg, true);
		}
		catch (const std::ifstream::failure& e) {
			printColoredMessage("������ ��� ������ � ������: " + std::string(e.what()), CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
		catch (...) {
			printColoredMessage("����������� ������.", CONSOLE_RED);
			return std::make_tuple(cv::Mat(), std::string(), start, false);
		}
	}

	void loadImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory) {
		try {
			// ��������� � ������� ����, ���� ����������
			checkAndCreatePaths({ {outputDirectory, binaryPath} });

			std::ifstream file(binaryPath, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				printColoredMessage("������: �� ������� ������� ���� " + binaryPath, CONSOLE_RED);
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
						printColoredMessage("������ ��� ������ �����������. ��������� ������� ����� " + binaryPath + " � ��������� �������.", CONSOLE_RED);
						return;
					}


					fs::path outputPath = fs::path(outputDirectory) / fs::path(imageName);
					fs::path filePathExceptFilename = outputPath.parent_path();

					if (!fs::exists(filePathExceptFilename)) {
						try {
							fs::create_directories(filePathExceptFilename);
						}
						catch (const std::exception& e) {
							printColoredMessage("�� ������� ������� ���������� " + filePathExceptFilename.string() + " ��� �����������: " + std::string(e.what()), CONSOLE_RED);
							// ������� ������ � ������� ����������
							if (!fs::exists(outputDirectory)) {
								try {
									fs::create_directories(outputDirectory);
								}
								catch (const std::exception& e) {
									printColoredMessage("�� ������� ������� " + outputDirectory + ": " + e.what(), CONSOLE_RED);
									return; // ��������� ���������� �������, ���� �� ������� ������� ����������
								}
								catch (...) {
									printColoredMessage("����������� ������ ��� �������� + outputDirectory", CONSOLE_RED);
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
						printColoredMessage("������: �� ������� ������������ ����������� " + outputPath.string(), CONSOLE_RED);
					}
					else {
						printColoredMessage("������������ ���� " + outputPath.string(), CONSOLE_GREEN);
						start = newStart;
					}
				}
				catch (const std::exception& e) {
					printColoredMessage("������ ��� ��������� �����������: " + std::string(e.what()), CONSOLE_RED);
				}
				catch (...) {
					printColoredMessage("����������� ������ ��� ��������� �����������.", CONSOLE_RED);
				}
			}
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������.", CONSOLE_RED);
		}
	}

	void deleteEntry(const std::filesystem::directory_entry& entry) {
		const std::string filePath = entry.path().string();
		try {
			if (entry.is_regular_file()) {
				if (std::filesystem::remove(filePath)) {
					printColoredMessage("���� " + filePath + " ������� ������.", CONSOLE_GREEN);
				}
				else {
					printColoredMessage("������: �� ������� ������� ���� " + filePath, CONSOLE_RED);
				}
			}
			else if (entry.is_directory()) {
				if (std::filesystem::remove_all(filePath)) {
					printColoredMessage("����� " + filePath + " ������� �������.", CONSOLE_GREEN);
				}
				else {
					printColoredMessage("������: �� ������� ������� ����� " + filePath, CONSOLE_RED);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			printColoredMessage("������ �������� ������� ��� �������� " + filePath + ": " + e.what(), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������ ��� �������� " + filePath + ": " + e.what(), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������ ��� �������� " + filePath, CONSOLE_RED);
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
					printColoredMessage("������ �������� ������� ��� ��������� " + entry.path().string() + ": " + e.what(), CONSOLE_RED);
				}
				catch (const std::exception& e) {
					printColoredMessage("����� ������ ��� ��������� " + entry.path().string() + ": " + e.what(), CONSOLE_RED);
				}
				catch (...) {
					printColoredMessage("����������� ������ ��� ��������� " + entry.path().string(), CONSOLE_RED);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			printColoredMessage("������ �������� ������� ��� �������� �� " + directoryPath + ": " + e.what(), CONSOLE_RED);
		}
		catch (const std::exception& e) {
			printColoredMessage("����� ������ ��� �������� �� " + directoryPath + ": " + e.what(), CONSOLE_RED);
		}
		catch (...) {
			printColoredMessage("����������� ������ ��� �������� �� " + directoryPath, CONSOLE_RED);
		}
	}

	bool checkAndCreatePaths(const Params& params) {
		bool createdAny = false;
		for (const auto& path : params.paths) {
			fs::path fsPath(path);

			try {
				// ���������, �������� �� ���� ������ ��� �����������
				if (fsPath.has_extension()) {
					// ��� ����, ������� ��� ������������ ����������
					fs::path parentPath = fsPath.parent_path();
					if (!parentPath.empty() && !fs::exists(parentPath)) {
						fs::create_directories(parentPath);
					}
					// ������� ����, ���� �� �� ����������
					if (!fs::exists(fsPath)) {
						std::ofstream file(fsPath);
						if (file) {
							createdAny = true;
							if (params.verbose) {
								printColoredMessage("������ ����: " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (params.verbose) {
								printColoredMessage("������ �������� �����: " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (params.verbose) {
							printColoredMessage("���� ��� ����������: " + fsPath.string(), CONSOLE_YELLOW);
						}
					}
				}
				else {
					// ��� ����������, ������� ��� ����������� ����������
					if (!fs::exists(fsPath)) {
						if (fs::create_directories(fsPath)) {
							createdAny = true;
							if (params.verbose) {
								printColoredMessage("������� ���������� " + fsPath.string(), CONSOLE_GREEN);
							}
						}
						else {
							if (params.verbose) {
								printColoredMessage("�� ������� ������� ���������� " + fsPath.string(), CONSOLE_RED);
							}
						}
					}
					else {
						if (params.verbose) {
							printColoredMessage("���������� " + fsPath.string() + " ��� ����������", CONSOLE_YELLOW);
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				printColoredMessage("������ �������� �������: " + std::string(e.what()), CONSOLE_RED);
			}
			catch (const std::exception& e) {
				printColoredMessage("����� ������: " + std::string(e.what()), CONSOLE_RED);
			}
			catch (...) {
				printColoredMessage("����������� ������ ��� ��������� ����: " + fsPath.string(), CONSOLE_RED);
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