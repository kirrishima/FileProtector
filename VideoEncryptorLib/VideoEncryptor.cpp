#include "pch.h"
#include "VideoEncryptor.h"

namespace fs = std::filesystem;

const std::string VideoEncryptor::DEFAULT_KEY = STR_DEFAULT_KEY;
const std::string VideoEncryptor::DEFAULT_BASE_DIRECTORY = STR_DEFAULT_BASE_DIRECTORY;
const std::string VideoEncryptor::DEFAULT_INPUT_FOLDER = STR_DEFAULT_INPUT_FOLDER;
const std::string VideoEncryptor::DEFAULT_ENCRYPTED_FOLDER = STR_DEFAULT_ENCRYPTED_FOLDER;
const std::string VideoEncryptor::DEFAULT_DECRYPTED_FOLDER = STR_DEFAULT_DECRYPTED_FOLDER;

VideoEncryptor::VideoEncryptor()
	: key(STR_DEFAULT_KEY), baseDirectory(STR_DEFAULT_BASE_DIRECTORY),
	inputFolder(STR_DEFAULT_INPUT_FOLDER), encryptedFolder(STR_DEFAULT_ENCRYPTED_FOLDER),
	decryptedFolder(STR_DEFAULT_DECRYPTED_FOLDER), shift(INT_DEFAULT_SHIFT),
	chunkSize(INT_DEFAULT_CHUNK_SIZE), shouldEncryptDecryptedFolder(BOOL_DEFAULT_SHOULD_ENCRYPT_DECRYPTED_FOLDER),
	deleteDecryptedFiles(BOOL_DEFAULT_SHOULD_DELETE_DECRYPTED_FILES) {
}

VideoEncryptor::VideoEncryptor(std::string key, std::string baseDirectory, std::string inputFolder, std::string encryptedFolder, std::string decryptedFolder, short shift)
	: key(std::move(key)),
	baseDirectory(std::move(baseDirectory)),
	inputFolder(std::move(inputFolder)),
	encryptedFolder(std::move(encryptedFolder)),
	decryptedFolder(std::move(decryptedFolder)),
	shift(shift),
	chunkSize(INT_DEFAULT_CHUNK_SIZE)
{
	this->shouldEncryptDecryptedFolder = DEFAULT_ENCRYPT_DECRYPTED_FOLDER;
	this->deleteDecryptedFiles = DEFAULT_DELETE_DECRYPTED_FILES;
}

std::string VideoEncryptor::encryptFilename(const std::string& filename, short shift, short maxShift) const
{
	return RCC::encryptFilename(string_to_wstring(filename), shift, maxShift);
}

std::vector<char> VideoEncryptor::readPartialFile(const std::string& filePath, std::streamsize size) const
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file.");
	}

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	buffer.resize(file.gcount());
	file.close();
	return buffer;
}

void VideoEncryptor::writePartialFile(const std::string& filePath, const std::vector<char>& data, std::streampos pos) const
{
	std::ofstream file(filePath, std::ios::binary | std::ios::in | std::ios::out);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file.");
	}

	file.seekp(pos);
	file.write(data.data(), data.size());
	file.close();
}

// xorEncryptDecrypt (обновлённая)
void VideoEncryptor::xorEncryptDecrypt(char* data, size_t len, const std::string& key, size_t& keyIndex) const {
	if (key.empty() || len == 0) return;
	const size_t keyLen = key.size();
	const char* keyPtr = key.data();
	for (size_t i = 0; i < len; ++i) {
		data[i] ^= keyPtr[keyIndex];
		if (++keyIndex == keyLen) keyIndex = 0;
	}
}

// encryptMp4 (обновлённая)
void VideoEncryptor::encryptMp4() const
{
	auto encryptFilesInDirectory = [&](const std::string& directoryPath)
		{
			std::string encryptedDirectoryPath = (fs::path(baseDirectory) / encryptedFolder).string();

			try {
				fs::create_directories(encryptedDirectoryPath);
			}
			catch (const fs::filesystem_error& e) {
				std::cerr << "Ошибка при создании директории: " << e.what() << std::endl;
				return;
			}
			catch (const std::exception& e) {
				std::cerr << "Неизвестная ошибка при создании директории: " << e.what() << std::endl;
				return;
			}

			// Оптимальный буфер: 8 MB (можно менять)
			const size_t BUFFER_SIZE = 8 * 1024 * 1024;
			std::vector<char> buffer(BUFFER_SIZE);

			for (const auto& entry : fs::directory_iterator(directoryPath))
			{
				std::string failedFileName = "Не удалось получить название файла";
				try
				{
					if (entry.is_regular_file())
					{
						std::string encryptingFilePath = entry.path().string();
						std::cout << "Шифруется " << encryptingFilePath << "..." << std::endl;

						std::string filename = failedFileName = entry.path().filename().string();
						std::string encryptedFilePath = (fs::path(encryptedDirectoryPath) /
							encryptFilename(filename, shift)).string();

						fs::path tmpPath = fs::path(encryptedFilePath + ".tmp");

						std::ifstream src(encryptingFilePath, std::ios::binary);
						if (!src.is_open())
							throw std::runtime_error("Не удалось открыть исходный файл.");

						std::ofstream dst(tmpPath.string(), std::ios::binary | std::ios::trunc);
						if (!dst.is_open())
							throw std::runtime_error("Не удалось создать временный файл назначения.");

						// Шифруем весь файл блоками
						size_t keyIndex = 0; // keep keystream position for the file
						while (src.read(buffer.data(), static_cast<std::streamsize>(BUFFER_SIZE)) || src.gcount() > 0) {
							std::streamsize bytesRead = src.gcount();
							if (bytesRead <= 0) break;
							xorEncryptDecrypt(buffer.data(), static_cast<size_t>(bytesRead), key, keyIndex);
							dst.write(buffer.data(), bytesRead);
						}

						src.close();
						dst.close();

						fs::rename(tmpPath, encryptedFilePath);

						std::cout << "Файл успешно зашифрован и сохранен как " << encryptedFilePath << std::endl;

						// Удаляем исходный файл
						if (fs::remove(encryptingFilePath))
							std::cout << "Файл " << encryptingFilePath << " удален." << std::endl;
						else
							std::cerr << "ОШИБКА: Не удалось удалить исходный файл " << filename << std::endl;
					}
				}
				catch (const fs::filesystem_error& e) {
					std::cerr << "ОШИБКА файловой системы при обработке файла " << failedFileName
						<< ": " << e.what() << std::endl;
				}
				catch (const std::ios_base::failure& e) {
					std::cerr << "ОШИБКА ввода-вывода при обработке файла " << failedFileName
						<< ": " << e.what() << std::endl;
				}
				catch (const std::exception& e) {
					std::cerr << "ОШИБКА при шифровании видео " << failedFileName
						<< ": " << e.what() << std::endl;
				}
				catch (...) {
					std::cerr << "Неизвестная ошибка при шифровании видео " << failedFileName << std::endl;
				}

				std::cout << std::endl;
			}
		};

	try {
		encryptFilesInDirectory((fs::path(baseDirectory) / inputFolder).string());

		if (VideoEncryptor::shouldEncryptDecryptedFolder) {
			encryptFilesInDirectory((fs::path(baseDirectory) / decryptedFolder).string());
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Ошибка при выполнении функции шифрования: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Неизвестная ошибка при выполнении функции шифрования" << std::endl;
	}
}

void VideoEncryptor::decryptMp4() const {
	std::string decryptedDirectoryPath = (fs::path(baseDirectory) / decryptedFolder).string();

	try {
		fs::create_directories(decryptedDirectoryPath);
	}
	catch (const std::exception& e) {
		std::cerr << "Ошибка при создании директории: " << e.what() << std::endl;
		return;
	}

	const std::string encryptedDirectoryPath = (fs::path(baseDirectory) / encryptedFolder).string();
	const size_t BUFFER_SIZE = 8 * 1024 * 1024;
	std::vector<char> buffer(BUFFER_SIZE);

	for (const auto& entry : fs::directory_iterator(encryptedDirectoryPath)) {
		if (!entry.is_regular_file()) continue;

		const std::string encryptedFilePath = entry.path().string();
		const std::string fileName = entry.path().filename().string();
		const std::string decryptedFilePath =
			(fs::path(decryptedDirectoryPath) / encryptFilename(fileName, -shift)).string();

		std::cout << "Расшифровывается файл: " << encryptedFilePath << std::endl;

		try {
			std::ifstream in(encryptedFilePath, std::ios::binary);
			if (!in.is_open()) {
				std::cerr << "Не удалось открыть файл " << encryptedFilePath << std::endl;
				continue;
			}

			std::ofstream out(decryptedFilePath, std::ios::binary | std::ios::trunc);
			if (!out.is_open()) {
				std::cerr << "Не удалось создать файл " << decryptedFilePath << std::endl;
				continue;
			}

			size_t keyIndex = 0; // IMPORTANT: one keystream position per file
			while (in.read(buffer.data(), static_cast<std::streamsize>(BUFFER_SIZE)) || in.gcount() > 0) {
				std::streamsize got = in.gcount();
				if (got <= 0) break;
				xorEncryptDecrypt(buffer.data(), static_cast<size_t>(got), key, keyIndex);
				out.write(buffer.data(), got);
			}

			std::cout << "Файл сохранён как: " << decryptedFilePath << std::endl;

			if (deleteDecryptedFiles) {
				if (fs::remove(encryptedFilePath)) {
					std::cout << "Исходный файл удалён: " << encryptedFilePath << std::endl;
				}
				else {
					std::cerr << "Ошибка: не удалось удалить " << encryptedFilePath << std::endl;
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Ошибка при расшифровке файла " << fileName << ": " << e.what() << std::endl;
		}

		std::cout << std::endl;
	}
}
