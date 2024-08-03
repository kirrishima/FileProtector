#include "stdafx.h"
#include "config.h"
#include <format>

const std::string ConfigHandler::defaultConfigFilePath = "config.json";

// ���������� ������� ������������
json ConfigHandler::getDefaultConfig() {
	return json{
		{"Paths", {
			{"images_base_directory", "IMG"},
			{"images_save_from_path", "Upload"},
			{"images_save_to_path", "Saves"},
			{"images_recovered_directory", "Recovered"},
			{"video_base_directory", "VIDS"},
			{"video_input_directory", "Input"},
			{"video_encrypted_directory", "Corrupted"},
			{"video_recovered_directory", "Fixed"},
		}},
		{"Files", {
			{"binary_path", "data.bin"},
			{"hash_file_path", "hash.txt"}
		}}
	};
}

// ������� ���� ������������ � �������� �����������
void ConfigHandler::createDefaultConfigFile(const std::string& filePath) {
	json defaultConfig = getDefaultConfig();
	std::ofstream configFile(filePath);
	if (configFile.is_open()) {
		configFile << defaultConfig.dump(4); // ������ � ��������� ��� �������� ������
		configFile.close();
		printColoredMessage("���������������� ���� ��� ������ �: " + filePath, CONSOLE_GREEN);
	}
	else {
		printColoredMessage("������: �� ������� ������� ���������������� ����!", CONSOLE_DARK_RED);
	}
}

// ��������� ���� ������������
json ConfigHandler::loadConfig(const std::string& filePath) {
	std::ifstream configFile(filePath);
	if (configFile.is_open()) {
		try {
			json config;
			configFile >> config;
			if (!validateConfig(config))
			{
				printColoredMessage("�� �������� ���������������� ����, ������������ ��������� �� ���������.", CONSOLE_DARK_RED);
				return getDefaultConfig();
			}
			printColoredMessage("������������ ��������� �� " + std::filesystem::absolute(filePath).string(), CONSOLE_GREEN);
			return config;
		}
		catch (const nlohmann::json::parse_error& e) {
			printColoredMessage(std::format("JSON parse error: {}", e.what()), CONSOLE_DARK_RED);
			return getDefaultConfig();
		}
		catch (const std::exception& e) {
			printColoredMessage(std::format("��������� ����������� ������: {}", e.what()), CONSOLE_DARK_RED);
			return getDefaultConfig();
		}
	}
	else {
		if (!std::filesystem::exists(filePath)) {
			printColoredMessage("���������������� ���� �� ������, �� ����� ������ � ����������� �� ���������.", CONSOLE_DARK_YELLOW);
			createDefaultConfigFile(std::filesystem::absolute(filePath).string());
		}
		else {
			printColoredMessage("�� ������� ������� ���������������� ����, ������������ ��������� �� ���������.", CONSOLE_DARK_YELLOW);
		}
		return getDefaultConfig();
	}
}

std::string ConfigHandler::getStringFromConfig(const json& config, const std::string& path, const std::string& defaultValue) {
	// ��������� ���� �� ����������
	size_t delimiterPos = path.find('.');
	if (delimiterPos == std::string::npos) {
		// ���� ��� �����������, ���� �������� ����� � �����
		if (config.contains(path) && config[path].is_string()) {
			return config[path];
		}
	}
	else {
		// ���� ���� �����������, ������������ ��������� �������
		std::string parentKey = path.substr(0, delimiterPos);
		std::string childKey = path.substr(delimiterPos + 1);

		if (config.contains(parentKey) && config[parentKey].is_object()) {
			return getStringFromConfig(config[parentKey], childKey, defaultValue);
		}
	}
	return defaultValue;
}

// ������� ��� �������� ��������� JSON � ��������
bool ConfigHandler::validateConfig(const json& config) {

	// �������� ������� ����������� ������ � �� �����
	if (!config.contains("Paths") || !config["Paths"].is_object() ||
		!config.contains("Files") || !config["Files"].is_object()) {
		return false;
	}

	const json& paths = config["Paths"];
	const json& files = config["Files"];

	// �������� ������ � Paths
	std::vector<std::string> pathsKeys = {
		"images_base_directory", "images_save_from_path", "images_save_to_path",
		"images_recovered_directory", "video_base_directory", "video_input_directory",
		"video_encrypted_directory", "video_recovered_directory"
	};

	for (const auto& key : pathsKeys) {
		if (!paths.contains(key) || !paths[key].is_string() || !imghider::isValidFileName(paths[key])) {
			return false;
		}
	}

	// �������� ������ � Files
	std::vector<std::string> filesKeys = { "binary_path", "hash_file_path" };

	for (const auto& key : filesKeys) {
		if (!files.contains(key) || !files[key].is_string() || !imghider::isValidFileName(files[key])) {
			return false;
		}
	}

	return true;
}