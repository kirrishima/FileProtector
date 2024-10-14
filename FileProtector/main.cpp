#include "stdafx.h"
#include "regex"
#include <limits>

//#define DISABLE_PSWD

std::string imagesBaseDirectory;
std::string imagesSaveFromPath;
std::string imagesSaveToPath;
std::string imagesRecoveredDirectory;
std::string binaryPath;
std::string hashFilePath;

std::string videoBaseDirectory;
std::string videoInputPath;
std::string videoEncryptedPath;
std::string videoRecoveredDirectory;

std::vector<std::string> paths;
std::unique_ptr<VideoEncryptor> VE;

static const int ConsoleDefaultHeight = GetConsoleWindowHeight();


bool authenticateUser() {
#ifndef DISABLE_PSWD
	std::cout << "Введите пароль: ";
	std::string pswd;
	std::getline(std::cin, pswd);
	if (pswd != PASSWORD) {

		std::cout << "Добро пожаловать!\nСписок просмотра:\nhttps://open.spotify.com/track/5ucIAerBlXCrES9RNEGNzH?si=30a9324a1cd34b31\n";
		system("pause");
		return false;
	}
#endif
	return true;
}

void initializeDirectories(const std::vector<std::string>& paths, bool verbose = false) {
	printColoredMessage("Создаются все необходимы папки, если их нет. . . \n\n", CONSOLE_GREEN);
	imghider::checkAndCreatePaths(paths, verbose);
}

void displayMainMenu();
void showDeletionMenu();
void handleUserInput(int userInput);
void printHelp();
int getUserChoice(bool shouldPrintHelp = false);
void configureFromJson();

int main() {
	ResizeConsole(36, 120, 10000, 120);

	std::setlocale(LC_ALL, "ru_RU");
	SetConsoleCP(1251);

	if (!authenticateUser()) {
		return 1;
	}

#if defined(_WIN32) || defined(_WIN64)
	std::system("cls");
#else
	std::system("clear");
#endif

	configureFromJson();
	printColoredMessage("\nНЕ УДАЛЯЙТЕ ПАПКИ " + binaryPath + " И " + videoEncryptedPath + " И ИХ СОДЕРЖИМОЕ ВО ИЗБЕЖАНИЕ ПОТЕРИ ИНФОРМАЦИИ\n", CONSOLE_RED);

	initializeDirectories(paths);

	int userInput;
	std::cout << std::endl;
	do {
		displayMainMenu();
		userInput = getUserChoice(true);
		handleUserInput(userInput);
	} while (true);

	system("pause");
	return 0;
}

void configureFromJson() {
	std::cout << std::endl;
	json config = ConfigHandler::loadConfig(ConfigHandler::DEFAULT_CONFIG_FILEPATH);

	const json& paths = config["Paths"];
	const json& files = config["Files"];

	imagesBaseDirectory = paths["images_base_directory"].get<std::string>();
	imagesSaveFromPath = (std::filesystem::path(imagesBaseDirectory) / paths["images_save_from_path"].get<std::string>()).string();
	imagesSaveToPath = (std::filesystem::path(imagesBaseDirectory) / paths["images_save_to_path"].get<std::string>()).string();
	imagesRecoveredDirectory = (std::filesystem::path(imagesBaseDirectory) / paths["images_recovered_directory"].get<std::string>()).string();

	binaryPath = (std::filesystem::path(imagesSaveToPath) / files["binary_path"].get<std::string>()).string();
	hashFilePath = (std::filesystem::path(imagesSaveToPath) / files["hash_file_path"].get<std::string>()).string();

	VE = std::make_unique<VideoEncryptor>(
		imghider::ENCRYPTING_KEY,
		paths["video_base_directory"].get<std::string>(),
		paths["video_input_directory"].get<std::string>(),
		paths["video_encrypted_directory"].get<std::string>(),
		paths["video_recovered_directory"].get<std::string>(),
		imghider::RCC_Shift
	);

	videoBaseDirectory = VE->getBaseDirectory();
	videoInputPath = (std::filesystem::path(videoBaseDirectory) / VE->getInputFolder()).string();
	videoEncryptedPath = (std::filesystem::path(videoBaseDirectory) / VE->getEncryptedFolder()).string();
	videoRecoveredDirectory = (std::filesystem::path(videoBaseDirectory) / VE->getDecryptedFolder()).string();

	::paths = { imagesSaveFromPath, imagesSaveToPath, binaryPath, imagesRecoveredDirectory, hashFilePath,
		videoInputPath, videoEncryptedPath, videoRecoveredDirectory };
}

void handleUserInput(int userInput) {
	switch (userInput) {
	case 1:
		std::cout << "\n";
		initializeDirectories(paths, true);
		break;
	case 2:
		imghider::saveImagesToBinary(imagesSaveFromPath, binaryPath, hashFilePath);
		std::cout << std::endl;
		break;
	case 3:
		imghider::loadAndSaveImagesFromBinary(binaryPath, imagesRecoveredDirectory);
		std::cout << std::endl;
		break;
	case 4:
	{
		std::string searchFilename;
		printColoredMessage("\nВведите название или часть названия для поиска: ", CONSOLE_BLUE, "");
		std::getline(std::cin, searchFilename);
		if (!imghider::isValidFileName(searchFilename) || searchFilename.empty())
		{
			printColoredMessage("Некорректное имя файла. Название не должно содержать след. символов: " + imghider::INVALID_FILEPATH_CHARS, CONSOLE_RED);
			break;
		}
		printColoredMessage("\nИдет поиск...\n", CONSOLE_CYAN);
		imghider::findImage(binaryPath, searchFilename, nullptr, false, imagesRecoveredDirectory);
		break;
	}
	case 5:
		VE->encryptMp4();
		break;
	case 6:
		VE->decryptMp4();
		break;
	case 7:
		VE->setShouldEncryptDecryptedFolder(true);
		break;
	case 8:
		showDeletionMenu();
		break;
	case 9:
		configureFromJson();
		break;
	case 10:
		ConfigHandler::createDefaultConfigFile(ConfigHandler::DEFAULT_CONFIG_FILEPATH);
		break;
	case (int)'h':
		printHelp();
		break;
	case -1:
		exit(EXIT_SUCCESS);

	default:
		printColoredMessage("Некорректный ввод", CONSOLE_RED);
		break;
	}
}

void displayMainMenu() {
	printColoredMessage("\nГлавное меню:", CONSOLE_CYAN);
	std::cout << "0, q - выход\n";
	std::cout << "1) Создать все необходимые папки и файлы\n";
	std::cout << "2) Сохранить изображения из " << imagesSaveFromPath << std::endl;
	std::cout << "3) Загрузить сохраненные изображения из " << binaryPath << std::endl;
	std::cout << "4) Поиск по фрагменту названия в сохраненных изображениях (" << binaryPath << ')' << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "5) Зашифровать видео из " << videoInputPath << std::endl;
	std::cout << "6) Расшифровать видео из " << videoEncryptedPath << std::endl;
	std::cout << "7) Также шифровать расшифрованные файлы из " << videoInputPath << " при выборе 4 пункта и сохранить в " << videoEncryptedPath << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "8) Меню удаления файлов и папок\n";
	std::cout << "9) Перезагрузить файл конфигурации\n";
	std::cout << "10) Пересоздать файл конфигурации\n";
	std::cout << "h) Справка по программе\n";
}

int getUserChoice(bool shouldPrintHelp) {
	std::regex numberPattern("^[0-9]+$");
	while (true)
	{
		printColoredMessage("\nВведите ваш выбор: ", CONSOLE_BLUE, "");
		std::string userInput;
		std::getline(std::cin, userInput);

		if (userInput == "0" || userInput == "q") {
			return -1;
		}
		if (shouldPrintHelp && (userInput == "h" || userInput == "help")) {
			return 'h';
		}
		if (!std::regex_match(userInput, numberPattern)) {
			printColoredMessage("Некорректный ввод", CONSOLE_RED);
			continue;
		}

		int choice = -1;
		try {
			size_t pos;
			choice = std::stoi(userInput, &pos);
			if (pos != userInput.length()) {
				printColoredMessage("Некорректный ввод", CONSOLE_RED);
				continue;
			}
		}
		catch (std::invalid_argument&) {
			printColoredMessage("Некорректный ввод", CONSOLE_RED);
			continue;
		}
		catch (std::out_of_range&) {
			printColoredMessage("Некорректный ввод", CONSOLE_RED);
			continue;
		}
		return choice;
	}
}

void showDeletionMenu() {
	bool exitFlag = false;
	printColoredMessage("\nМеню удаления файлов и папок", CONSOLE_CYAN);

	while (!exitFlag) {
		std::cout << "\n0, q - возврат в главное меню\n";
		printColoredMessage("----- Изображения -----", CONSOLE_CYAN);
		std::cout << "1) Отчистить всю папку " << imagesBaseDirectory << std::endl;
		std::cout << "2) Удалить восстановленные изображения из " << imagesRecoveredDirectory << std::endl;
		std::cout << "3) Удалить сохраненные изображения из " << imagesSaveToPath << std::endl;
		std::cout << "4) Удалить несохраненные изображения из " << imagesSaveFromPath << std::endl;
		printColoredMessage("----- Видео -----", CONSOLE_CYAN);
		std::cout << "5) Удалить всё из " << videoBaseDirectory << std::endl;
		std::cout << "6) Удалить восстановленные видео из " << videoRecoveredDirectory << std::endl;
		std::cout << "7) Удалить зашифрованные видео из " << videoEncryptedPath << std::endl;
		std::cout << "8) Удалить несохраненные видео из " << videoInputPath << std::endl;
		printColoredMessage("----- Изображения + Видео -----", CONSOLE_CYAN);
		std::cout << "9) Удалить всё из " << imagesBaseDirectory + " И " + videoBaseDirectory << std::endl;
		std::cout << "10) Удалить восстановленные изображения и видео из " << imagesRecoveredDirectory + " И " + videoRecoveredDirectory << std::endl;
		std::cout << "11) Удалить сохраненные изображения из " << imagesSaveToPath + " И зашифрованные видео из " + videoEncryptedPath << std::endl;
		std::cout << "12) Удалить несохраненные изображения и видео из " << imagesSaveFromPath + " И " + videoInputPath << std::endl;

		int choice = getUserChoice();


		if (choice == -1)
		{
			exitFlag = true;
			continue;
		}

		while (choice < 1 || choice > 12)
		{
			printColoredMessage("Некорректный ввод", CONSOLE_RED);
			choice = getUserChoice();
			continue;
		}

		printColoredMessage("\nВНИМАНИЕ: ", CONSOLE_RED, "");
		printColoredMessage(" это действие нельзя будет отменить и данные будут удалены ", CONSOLE_CYAN, "");
		printColoredMessage("БЕЗВОЗВРАТНО", CONSOLE_RED, "");

		char confirm;
		do {
			printColoredMessage("\nЖелаете продолжить (y/n)? ", CONSOLE_CYAN, "");
			std::cin >> confirm;
			confirm = std::tolower(confirm);
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (confirm != 'y' && confirm != 'n');

		if (confirm == 'n') {
			printColoredMessage("Удаление было отменено", CONSOLE_YELLOW);
			continue;
		}

		std::cout << std::endl;

		switch (choice) {
		case 1:
			imghider::clearDirectory(imagesBaseDirectory, true);
			break;
		case 2:
			imghider::clearDirectory(imagesRecoveredDirectory, true);
			break;
		case 3:
			imghider::clearDirectory(imagesSaveToPath);
			break;
		case 4:
			imghider::clearDirectory(imagesSaveFromPath, true);
			break;
		case 5:
			imghider::clearDirectory(videoBaseDirectory, true);
			break;
		case 6:
			imghider::clearDirectory(videoRecoveredDirectory);
			break;
		case 7:
			imghider::clearDirectory(videoEncryptedPath);
			break;
		case 8:
			imghider::clearDirectory(videoInputPath);
			break;
		case 9:
			imghider::clearDirectory(imagesBaseDirectory, true);
			std::cout << std::endl;
			imghider::clearDirectory(videoBaseDirectory, true);
			break;
		case 10:
			imghider::clearDirectory(imagesRecoveredDirectory, true);
			std::cout << std::endl;
			imghider::clearDirectory(videoRecoveredDirectory, true);
			break;
		case 11:
			imghider::clearDirectory(imagesSaveToPath);
			std::cout << std::endl;
			imghider::clearDirectory(videoEncryptedPath);
			break;
		case 12:
			imghider::clearDirectory(imagesSaveFromPath, true);
			std::cout << std::endl;
			imghider::clearDirectory(videoInputPath, true);
			break;
		default:
			std::cout << "Некорректный ввод\n";
			break;
		}
	}
	printColoredMessage("\nВозврат в главное меню. . .", CONSOLE_CYAN);
}

void printHelp() {
	std::cout << "\n\n";
	printCentered("Программа для шифрования и сокрытия изображений и видео.\n", CONSOLE_GREEN);

	printColoredMessage("Эта программа позволяет вам:", CONSOLE_DARK_CYAN);
	std::cout << "1. Сохранить изображения в бинарный файл." << std::endl;
	std::cout << "2. Загрузить изображения из бинарного файла." << std::endl;
	std::cout << "3. Зашифровать и расшифровать видеофайлы." << std::endl;
	std::cout << "4. Удалять файлы и папки, связанные с процессом шифрования и сокрытия." << std::endl;
	std::cout << std::endl;

	printColoredMessage("Используемые пути и папки (задаются в config.json):", CONSOLE_DARK_CYAN);

	printColoredMessage("\nДля изображений:", CONSOLE_DARK_YELLOW);
	std::cout << " - Каталог для загрузки изображений: " << imagesSaveFromPath << std::endl;
	std::cout << " - Каталог для сохранения изображений в бинарном формате: " << binaryPath << std::endl;
	std::cout << " - Каталог для восстановления изображений: " << imagesRecoveredDirectory << std::endl;
	std::cout << " - Каталог для сохранения хэша изображений: " << hashFilePath << std::endl;
	printColoredMessage("\nДля видео:", CONSOLE_DARK_YELLOW);
	std::cout << " - Каталог для входных видеофайлов: " << videoInputPath << std::endl;
	std::cout << " - Каталог для сохранения зашифрованных видеофайлов: " << videoEncryptedPath << std::endl;
	std::cout << " - Каталог для восстановления расшифрованных видеофайлов: " << videoRecoveredDirectory << std::endl;
	std::cout << std::endl;

	printColoredMessage("Как пользоваться:", CONSOLE_DARK_CYAN);

	std::cout << " - Необходимо ввести число или символ, отражающее определенный номер одного из пунктов меню" << std::endl;
	std::cout << std::endl;

	printColoredMessage("Внимание:", CONSOLE_DARK_RED);
	std::cout << " - Данные из " << binaryPath << " не удаляются при восстановлении изображений, поэтому повторно шифровать восстановленные изображения не нужно\n";
	std::cout << " - Зашифрованные видеофайлы из " << videoEncryptedPath << " не удаляются при расшифровании, поэтому повторно шифровать их также не нужно\n";
	std::cout << " - При выборе 6 пункта главного меню - включение шифрования видео из папки с расшифрованными видео - данная настройка действует только в этот раз и не будет сохранена для последующих запусков\n";
	std::cout << " - Удаление папок и файлов необратимо. Будьте осторожны при выборе соответствующих пунктов меню." << std::endl;
	std::cout << " - Не удаляйте папки, содержащие важные файлы, такие как: " << binaryPath << " и " << videoEncryptedPath << " во избежание потери сохранённых данных." << std::endl;
	std::cout << std::endl;

	SET_CONSOLE_DARK_CYAN;
	system("pause");
	SET_CONSOLE_DEFAULT;
}
