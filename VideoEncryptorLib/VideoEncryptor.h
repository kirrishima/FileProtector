#ifndef VIDEO_ENCRYPTOR_H
#define VIDEO_ENCRYPTOR_H

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

constexpr auto STR_DEFAULT_KEY = "Force_AKA_Moonglow";
constexpr auto STR_DEFAULT_BASE_DIRECTORY = "VIDS";
constexpr auto STR_DEFAULT_INPUT_FOLDER = "Input";
constexpr auto STR_DEFAULT_ENCRYPTED_FOLDER = "Corrupted";
constexpr auto STR_DEFAULT_DECRYPTED_FOLDER = "Pure";
constexpr auto INT_DEFAULT_SHIFT = 1;
constexpr auto INT_DEFAULT_CHUNK_SIZE = 1024 * 1024;
constexpr auto BOOL_DEFAULT_SHOULD_ENCRYPT_DECRYPTED_FOLDER = false;
constexpr auto BOOL_DEFAULT_SHOULD_DELETE_DECRYPTED_FILES = false;

class VideoEncryptor
{
public:

	static const std::string DEFAULT_KEY;
	static const std::string DEFAULT_BASE_DIRECTORY;
	static const std::string DEFAULT_INPUT_FOLDER;
	static const std::string DEFAULT_ENCRYPTED_FOLDER;
	static const std::string DEFAULT_DECRYPTED_FOLDER;
	static constexpr int DEFAULT_SHIFT = INT_DEFAULT_SHIFT;
	static constexpr bool DEFAULT_ENCRYPT_DECRYPTED_FOLDER = BOOL_DEFAULT_SHOULD_ENCRYPT_DECRYPTED_FOLDER;
	static constexpr bool DEFAULT_DELETE_DECRYPTED_FILES = BOOL_DEFAULT_SHOULD_DELETE_DECRYPTED_FILES;

	VideoEncryptor();
	VideoEncryptor(std::string key, std::string baseDirectory, std::string inputFolder, std::string encryptedFolder, std::string decryptedFolder, short shift);

	std::string getKey() const { return key; }
	std::string getBaseDirectory() const { return baseDirectory; }
	std::string getInputFolder() const { return inputFolder; }
	std::string getEncryptedFolder() const { return encryptedFolder; }
	std::string getDecryptedFolder() const { return decryptedFolder; }
	int getShift() const { return shift; }
	bool getShouldEncryptDecryptedFolder() const { return shouldEncryptDecryptedFolder; }
	bool getDeleteDecryptedFiles() const { return deleteDecryptedFiles; }

	void setShouldEncryptDecryptedFolder(bool value) { shouldEncryptDecryptedFolder = value; }
	void setDeleteDecryptedFiles(bool value) { deleteDecryptedFiles = value; };

	void encryptMp4() const;
	void decryptMp4() const;
private:

	const std::string key;
	const std::string baseDirectory;
	const std::string inputFolder;
	const std::string encryptedFolder;
	const std::string decryptedFolder;
	const int shift;
	const std::streamsize chunkSize;

	bool shouldEncryptDecryptedFolder;
	bool deleteDecryptedFiles;

	std::string encryptFilename(const std::string& filename, short shift = 1, short maxShift = 10) const;
	std::vector<char> readPartialFile(const std::string& filePath, std::streamsize size) const;
	void writePartialFile(const std::string& filePath, const std::vector<char>& data, std::streampos pos) const;
	void xorEncryptDecrypt(char* data, size_t len, const std::string& key, size_t& keyIndex) const;
};


namespace RCC { // Хранит функцию для циклического сдвига символов строки, при этом символы никогда не выйдут за пределы алфавита
	std::string encryptFilename(std::wstring filename, short shift = 1, short maxShift = 15);
}
#endif // VIDEO_ENCRYPTOR_H

std::string wstring_to_string(const std::wstring& wstr);
std::wstring string_to_wstring(const std::string& str);