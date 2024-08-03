#ifndef VIDEO_ENCRYPTOR_H
#define VIDEO_ENCRYPTOR_H
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

// Определения для проверки символов кириллицы
#define IS_CYRILLIC(x) (('а' <= (x) && (x) <= 'я') || ('А' <= (x) && (x) <= 'Я'))
#define IS_LOWER_CYRILLIC(x) ('а' <= (x) && (x) <= 'я')

#define STR_DEFAULT_KEY "Force_AKA_Moonglow"
#define STR_DEFAULT_BASE_DIRECTORY "VIDS"
#define STR_DEFAULT_INPUT_FOLDER "Input"
#define STR_DEFAULT_ENCRYPTED_FOLDER "Corrupted"
#define STR_DEFAULT_DECRYPTED_FOLDER "Pure"
#define INT_DEFAULT_SHIFT 1
#define INT_DEFAULT_CHUNK_SIZE 1024 * 1024
#define BOOL_DEFAULT_SHOULD_ENCRYPT_DECRYPTED_FOLDER false
#define BOOL_DEFAULT_SHOULD_DELETE_DECRYPTED_FILES false

class VideoEncryptor
{
public:

	// Константы 
	static const std::string DEFAULT_KEY;
	static const std::string DEFAULT_BASE_DIRECTORY;
	static const std::string DEFAULT_INPUT_FOLDER;
	static const std::string DEFAULT_ENCRYPTED_FOLDER;
	static const std::string DEFAULT_DECRYPTED_FOLDER;
	static constexpr int DEFAULT_SHIFT = INT_DEFAULT_SHIFT;
	static constexpr bool DEFAULT_ENCRYPT_DECRYPTED_FOLDER = BOOL_DEFAULT_SHOULD_ENCRYPT_DECRYPTED_FOLDER;
	static constexpr bool DEFAULT_DELETE_DECRYPTED_FILES = BOOL_DEFAULT_SHOULD_DELETE_DECRYPTED_FILES;
	// Конструкторы
	VideoEncryptor();
	VideoEncryptor(std::string key, std::string baseDirectory, std::string inputFolder, std::string encryptedFolder, std::string decryptedFolder, short shift);

	// Геттеры
	std::string getKey() const { return key; }
	std::string getBaseDirectory() const { return baseDirectory; }
	std::string getInputFolder() const { return inputFolder; }
	std::string getEncryptedFolder() const { return encryptedFolder; }
	std::string getDecryptedFolder() const { return decryptedFolder; }
	int getShift() const { return shift; }
	bool getShouldEncryptDecryptedFolder() const { return shouldEncryptDecryptedFolder; }
	bool getDeleteDecryptedFiles() const { return deleteDecryptedFiles; }

	// Сеттеры
	void setShouldEncryptDecryptedFolder(bool value) { shouldEncryptDecryptedFolder = value; }
	void setDeleteDecryptedFiles(bool value) { deleteDecryptedFiles = value; };

	// Методы
	void encryptMp4() const;
	void decryptMp4() const;
private:
	// Константы и параметры
	static bool initialized;
	const std::string key;
	const std::string baseDirectory;
	const std::string inputFolder;
	const std::string encryptedFolder;
	const std::string decryptedFolder;
	const int shift;
	const std::streamsize chunkSize;

	bool shouldEncryptDecryptedFolder;
	bool deleteDecryptedFiles;
	// Функции-члены	
	std::string encryptFilename(const std::string& filename, short shift = 1, short maxShift = 10) const;
	std::vector<char> readPartialFile(const std::string& filePath, std::streamsize size) const;
	void writePartialFile(const std::string& filePath, const std::vector<char>& data, std::streampos pos) const;
	void xorEncryptDecrypt(std::vector<char>& data, const std::string& key) const;
};


namespace RCC { // Хранит функцию для циклического сдвига символов строки, при этом символы никогда не выйдут за пределы алфавита
	std::string encryptFilename(std::wstring filename, short shift = 1, short maxShift = 15);
}
#endif // VIDEO_ENCRYPTOR_H

std::string wstring_to_string(const std::wstring& wstr);
std::wstring string_to_wstring(const std::string& str);