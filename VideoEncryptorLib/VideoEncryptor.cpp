#include "pch.h"
#include "VideoEncryptor.h"


//// Инициализация статических констант
const std::string VideoEncryptor::DEFAULT_KEY = STR_DEFAULT_KEY;
const std::string VideoEncryptor::DEFAULT_BASE_DIRECTORY = STR_DEFAULT_BASE_DIRECTORY;
const std::string VideoEncryptor::DEFAULT_INPUT_FOLDER = STR_DEFAULT_INPUT_FOLDER;
const std::string VideoEncryptor::DEFAULT_ENCRYPTED_FOLDER = STR_DEFAULT_ENCRYPTED_FOLDER;
const std::string VideoEncryptor::DEFAULT_DECRYPTED_FOLDER = STR_DEFAULT_DECRYPTED_FOLDER;

VideoEncryptor::VideoEncryptor()
	: key(DEFAULT_KEY), baseDirectory(DEFAULT_BASE_DIRECTORY),
	inputFolder(DEFAULT_INPUT_FOLDER), encryptedFolder(DEFAULT_ENCRYPTED_FOLDER),
	decryptedFolder(DEFAULT_DECRYPTED_FOLDER), shift(DEFAULT_SHIFT),
	chunkSize(1024 * 1024), shouldEncryptDecryptedFolder(DEFAULT_ENCRYPT_DECRYPTED_FOLDER),
	deleteDecryptedFiles(DEFAULT_DELETE_DECRYPTED_FILES) {

}

VideoEncryptor::VideoEncryptor(std::string key, std::string baseDirectory, std::string inputFolder, std::string encryptedFolder, std::string decryptedFolder, short shift)
	: key(std::move(key)),
	baseDirectory(std::move(baseDirectory)),
	inputFolder(std::move(inputFolder)),
	encryptedFolder(std::move(encryptedFolder)),
	decryptedFolder(std::move(decryptedFolder)),
	shift(shift),
	chunkSize(1024 * 1024)
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

void VideoEncryptor::xorEncryptDecrypt(std::vector<char>& data, const std::string& key) const
{
	size_t keyLen = key.size();
	for (size_t i = 0; i < data.size(); ++i)
	{
		data[i] ^= key[i % keyLen];
	}
}

void VideoEncryptor::encryptMp4() const
{
	auto encryptFilesInDirectory = [&](const std::string& directoryPath)
		{
			std::string encryptedDirectoryPath = (std::filesystem::path(baseDirectory) / encryptedFolder).string();

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

			for (const auto& entry : fs::directory_iterator(directoryPath))
			{
				std::string failedFileName = "Не удалось получить название файла";
				try
				{
					if (entry.is_regular_file())
					{
						try {
							std::string encryptingFilePath = entry.path().string();
							std::cout << "Шифруется " << encryptingFilePath << ". . ." << std::endl;
							std::string filename = failedFileName = entry.path().filename().string();
							std::string encryptedFilePath = (std::filesystem::path(encryptedDirectoryPath) / encryptFilename(filename, shift)).string();

							// Read the first 1 MB of the input file
							std::vector<char> fileData = readPartialFile(encryptingFilePath, chunkSize);

							// Encrypt the first 1 MB
							xorEncryptDecrypt(fileData, key);

							// Copy the original file to the encrypted file
							fs::copy_file(encryptingFilePath, encryptedFilePath, fs::copy_options::overwrite_existing);

							// Write the encrypted chunk back to the new file
							writePartialFile(encryptedFilePath, fileData, 0);
							std::cout << "Файл успешно зашифрован и сохранен как " << encryptedFilePath << std::endl;
							// Delete the original file
							if (fs::remove(encryptingFilePath)) {
								std::cout << "Файл " << encryptingFilePath << " удален." << std::endl;
							}
							else {
								std::cerr << "ОШИБКА: Не удалось удалить исходный файл " << filename << std::endl;
							}
						}
						catch (const fs::filesystem_error& e) {
							std::cerr << "ОШИБКА файловой системы при обработке файла " << failedFileName << ": " << e.what() << std::endl;
						}
						catch (const std::ios_base::failure& e) {
							std::cerr << "ОШИБКА ввода-вывода при обработке файла " << failedFileName << ": " << e.what() << std::endl;
						}
						catch (const std::exception& e) {
							std::cerr << "ОШИБКА при шифровании видео " << failedFileName << ": " << e.what() << std::endl;
						}
						catch (...) {
							std::cerr << "Неизвестная ошибка при шифровании видео " << failedFileName << std::endl;
						}
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "ОШИБКА при обработке директории для файла " << failedFileName << ": " << e.what() << std::endl;
				}
				catch (...) {
					std::cerr << "Неизвестная ошибка при обработке директории для файла " << failedFileName << std::endl;
				}
				std::cout << std::endl << std::endl;
			}
		};

	try {
		// Обработка файлов в основной директории
		encryptFilesInDirectory((std::filesystem::path(baseDirectory) / inputFolder).string());

		// Обработка файлов в директории, куда восстанавливаются файлы
		if (VideoEncryptor::shouldEncryptDecryptedFolder) {
			encryptFilesInDirectory((std::filesystem::path(baseDirectory) / decryptedFolder).string());
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Ошибка при выполнении функции шифрования: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Неизвестная ошибка при выполнении функции шифрования" << std::endl;
	}
}


void VideoEncryptor::decryptMp4() const
{
	std::string decryptedDirectoryPath = (std::filesystem::path(baseDirectory) / decryptedFolder).string();
	try {
		fs::create_directories(decryptedDirectoryPath);
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Ошибка при создании директории: " << e.what() << std::endl;
		return;
	}
	catch (const std::exception& e) {
		std::cerr << "Неизвестная ошибка при создании директории: " << e.what() << std::endl;
		return;
	}

	std::string encryptedDirectoryPath = (std::filesystem::path(baseDirectory) / encryptedFolder).string();

	for (const auto& entry : fs::directory_iterator(encryptedDirectoryPath))
	{
		std::string failedFileName = "Не удалось получить название файла";
		try {
			if (entry.is_regular_file())
			{
				try {
					std::string encryptedFilePath = entry.path().string();
					std::cout << "Расшифровывается " << encryptedFilePath << ". . ." << std::endl;
					std::string fileName = failedFileName = entry.path().filename().string();
					std::string decryptedFilePath = (std::filesystem::path(decryptedDirectoryPath) / encryptFilename(fileName, -shift)).string();

					// Read the first 1 MB of the encrypted file
					std::vector<char> encryptedData = readPartialFile(encryptedFilePath, chunkSize);

					// Decrypt the first 1 MB
					xorEncryptDecrypt(encryptedData, key);

					// Copy the encrypted file to the decrypted file
					fs::copy_file(encryptedFilePath, decryptedFilePath, fs::copy_options::overwrite_existing);

					// Write the decrypted chunk back to the new file
					writePartialFile(decryptedFilePath, encryptedData, 0);

					std::cout << "Файл расшифрован и сохранен как " << decryptedFilePath << std::endl;

					if (deleteDecryptedFiles) {
						if (fs::remove(encryptedFilePath)) {
							std::cout << "Файл " << fileName << " удален из " << encryptedDirectoryPath << std::endl;
						}
						else {
							std::cerr << "Ошибка: Не удалось удалить исходный файл." << std::endl;
						}
					}
					//std::cout << std::endl;
				}
				catch (const fs::filesystem_error& e) {
					std::cerr << "Ошибка файловой системы при обработке файла " << failedFileName << ": " << e.what() << std::endl;
				}
				catch (const std::ios_base::failure& e) {
					std::cerr << "Ошибка ввода-вывода при обработке файла " << failedFileName << ": " << e.what() << std::endl;
				}
				catch (const std::exception& e) {
					std::cerr << "Ошибка при расшифровке видео " << failedFileName << ": " << e.what() << std::endl;
				}
				catch (...) {
					std::cerr << "Неизвестная ошибка при расшифровке видео " << failedFileName << std::endl;
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Ошибка при обработке директории для файла " << failedFileName << ": " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Неизвестная ошибка при обработке директории для файла " << failedFileName << std::endl;
		}
		std::cout << std::endl;
	}
}

