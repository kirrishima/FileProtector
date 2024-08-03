#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "nlohmann/json.hpp"

namespace imghider
{
	static const short RCC_Shift = 1; // File Name Encrypting Start Shift
	static constexpr int DEFAULT_CHUNK_SIZE = 1024 * 1024; // Size in bytes of the file to be encoded, 1 MB By default
	static const std::string ENCRYPTING_KEY = "Enter_YOUR_Key_Which_Will_Be_Used_In_Encrypring";
}

static const std::string PASSWORD = ""; // Spicify your password

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
