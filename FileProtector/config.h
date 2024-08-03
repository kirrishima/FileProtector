#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "nlohmann/json.hpp"

namespace imghider
{
	static const short RCC_Shift = 3;
	static constexpr int DEFAULT_CHUNK_SIZE = 1024 * 1024;
	static const std::string ENCRYPTING_KEY = "Force_AKA_Moonglow_Qpro_I_Love_Sadar_baby_Goat";
}


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
