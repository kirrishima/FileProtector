#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "nlohmann/json.hpp"
#include <set>

typedef unsigned char uchar;

namespace imghider
{
	static constexpr short RCC_Shift = 1; // Сдвиг за один шаг
	static constexpr int DEFAULT_CHUNK_SIZE = 1024 * 1024; // Размер кодируемого файла в байтах, по умолчанию 1 МБ
	static const std::string ENCRYPTING_KEY = "Enter_YOUR_Key_Which_Will_Be_Used_In_Encrypting";
	static const std::set<std::string> image_extensions = { ".jpg", ".jpeg", ".png", ".bmp", ".tif", ".webp" }; // Поддерживаемые форматы изображения
	static const std::string invalidChars = "\\/:*?\"<>|"; // Запрещенные символы в пути и названии файла
}

static const std::string PASSWORD = ""; // Пароль

using json = nlohmann::json;

class ConfigHandler {
public:
	static const std::string defaultConfigFilePath;
	static json getDefaultConfig();
	static bool validateConfig(const json& config);
	static void createDefaultConfigFile(const std::string& filePath);
	static json loadConfig(const std::string& filePath);
	std::string getStringFromConfig(const json& config, const std::string& path, const std::string& defaultValue);
};

#endif // !CONFIG_H
