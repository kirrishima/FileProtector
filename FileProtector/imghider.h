#ifndef IMGHIDER_H
#define IMGHIDER_H
#include <filesystem>
#include <string>
#include <vector>
#include "opencv2/core/mat.hpp"

namespace fs = std::filesystem;

namespace imghider {
	
	/**
	*@brief Выводит изображение на экран
	*
	*@param image объект: cv::Mat, представляющий изображение
	*@param displayName: Заголовок окна с изображением
	*@return Возвращает успешность выполнения кода
	**/
	bool displayImage(const cv::Mat& image, const std::string& displayName)noexcept;
	
	/**
	 * @brief Находит изображение и выводит на экран, если оно найдено
	 *
	 * @param binaryPath: файл, в котором будет происходить поиск
	 * @param searchFilename: название изображения для поиска
	 * @return Указывает, было ли найдено изображение
	 */
	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename) noexcept;
	
	/**
	 * @brief Находит и возвращает изображение
	 *
	 * @param binaryPath: файл, в котором будет происходить поиск
	 * @param searchFilename: название изображения для поиска
	 * @param foundImageName: название найденного изображения
	 * @param exactMatch: если false, будет искать все изображения с searchFilename в названии
	 * @return возвращает найденное изображение в случае успеха, иначе пустое 
	 */
	cv::Mat findImage(const std::string& binaryPath, const std::string& searchFilename, std::string* foundImageName, bool exactMatch = true, const std::string& outputDirectory = "") noexcept;

	/**
	 * @brief Сохраняет изображение и его метаданные в бинарный файл.
	 *
	 * @param binaryPath Путь к бинарному файлу, в который будет сохранено изображение.
	 * @param imageName Имя изображения, которое будет сохранено.
	 * @param image Изображение, которое будет сохранено.
	 *
	 *@return Возвращает успех сохранения
	 */
	bool saveImageToBinary(const std::string& binaryPath, const std::string& imageName, const cv::Mat& image);

	/**
	* @brief Записывает изображения в .bin файл
	*
	* @param directoryPath: директория, из которой будут браться изображения, после записи - удаляться
	* @param binaryPath: путь к бинарному файлу
	* @param hashFilePath: путь к файлу с хэшем названий изображений
	*/
	void saveImagesToBinary(const std::string& directoryPath, const std::string& binaryPath, const std::string& hashFilePath);
	
	/**
	 * @brief Сохраняет на диске переданное изображение.
	 *
	 * @param image: Изображение.
	 * @param imageName: Название изображения, с которым оно будет сохранено.
	 * @param outputDirectory: Путь, по которому будет сохранено изображение.
	 * @return Успешность сохранения.
	 */
	bool saveImage(const cv::Mat image, const fs::path& imageName, const fs::path& outputDirectory) noexcept;
	
	/**
	 * @brief Загружает изображение и его метаданные из бинарного файла.
	 *
	 * @param binaryPath: Путь к бинарному файлу, из которого будет загружаться изображение.
	 * @param start: Начальная позиция в бинарном файле, с которой начнется чтение.
	 * @return std::tuple<cv::Mat, std::string, int, bool> Кортеж, содержащий изображение,
	 * название изображения + расширение, позицию в файле после чтения и флаг успеха.
	 */
	std::tuple<cv::Mat, std::string, size_t, bool> loadImageFromBinary(const std::string& binaryPath, const size_t start);

	/**
	 * @brief Загружает изображения и их метаданные из бинарного файла и сохраняет их в указанной директории.
	 *
	 * @param binaryPath: Путь к бинарному файлу, из которого будут загружены изображения.
	 * @param outputDirectory: Директория, в которую будут сохранены загруженные изображения.
	 */
	void loadAndSaveImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory);

	/**
	 * @brief Удаляет файлы из заданной директории
	 *
	 * @param directoryPath: Директория, которую надо удалить.
	 */
	void clearDirectory(const std::string& directoryPath, bool deleteSubfolders = false, bool clearSubfolders = false);

	 /**
	  * @brief Создает указанные директории и файлы, если их не существует
	  *
	  * @param params: структура с параметрами, с полями paths (вектор строк, содержащий пути) и verbose (Флаг для включения/выключения подробного вывода)
	  * @return указывает, были ли созданы какие-либо директории или файлы
	  */
	bool checkAndCreatePaths(const std::vector<std::string>& paths, bool verbose = false);

	/**
	 * @brief  Проверяет, существует ли уже хэш названия изображения в файле
	 *
	 * @param hashFilePath: путь к файлу с хэшем
	 * @param fileHash: хэш для поиска
	 * @return возвращает результат поиска
	 */
	bool isNameHashInFile(const std::string& hashFilePath, const std::string& fileHash);

	/**
	 * @brief Записывает хэш в файл с хэшем
	 *
	 * @param hashFilePath: путь к файлу с хэшами
	 * @param hashValue: значение хэша
	 */
	void addNameHashToFile(const std::string& hashFilePath, const std::string& hashValue);

	// Функция для генерации уникального имени файла с добавлением текущей даты и времени
	/**
	 * @brief Генерирует уникальное имя файла с добавлением текущей даты и времени
	 *
	 * @param originalFilename: название файла для обработки, с расширением
	 */
	std::string getUniqueFilename(const std::string&);

	/**
	 * @brief Разрешает конфликт имен, когда в файле .bin уже существует файл с таким именем, путем изменения его имени или пропуска
	 *
	 * @param directoryPath: название файла для обработки, с расширением
	 * @param binaryPath: Путь к .bin файлу
	 * @param imagePath: Путь к изображению на диске
	 * @param hashFilePath: Путь к файлу с хэшем
	 * @param fileRelPath: Относительный путь изображения, относительно базовой папки для загрузки изображений. Может быть изменен в функции
	 * @param fileHash: Хэш файла, передается по ссылке, может быть изменен в функции
	 * @param image: Изображение
	 * 
	 * @return bool, нужно ли пропустить данное изображение
	 */
	bool resolveDuplicate(
		const std::string& directoryPath,
		const std::string& binaryPath,
		const std::string& imagePath,
		const std::string& hashFilePath,
		fs::path& fileRelPath,
		std::string& fileHash,
		const cv::Mat& image
	);

	/**
	 * @brief Удаляет пробелы в начале строки.
	 *
	 * Эта функция удаляет все пробелы, находящиеся в начале указанной строки.
	 *
	 * @param s Входная строка, из которой будут удалены пробелы в начале.
	 * @return Новая строка без пробелов в начале.
	 */
	std::string ltrim(const std::string& s);

	/**
	 * @brief Удаляет пробелы в конце строки.
	 *
	 * Эта функция удаляет все пробелы, находящиеся в конце указанной строки.
	 *
	 * @param s Входная строка, из которой будут удалены пробелы в конце.
	 * @return Новая строка без пробелов в конце.
	 */
	std::string rtrim(const std::string& s);

	/**
	 * @brief Удаляет пробелы в начале и конце строки.
	 *
	 * Эта функция удаляет все пробелы, находящиеся в начале и конце указанной строки.
	 *
	 * @param s Входная строка, из которой будут удалены пробелы в начале и конце.
	 * @return Новая строка без пробелов в начале и конце.
	 */
	std::string trim(const std::string& s);

	/**
	 * @brief Удаляет пробелы в начале и конце всех частей пути.
	 *
	 * Эта функция удаляет пробелы в начале и конце каждой части пути.
	 *
	 * @param path Входной путь, части которого будут обрезаны.
	 * @return Путь с обрезанными частями.
	 */
	fs::path trimPath(fs::path path);

	/**
	 * @brief Добавляет суффикс к имени файла, если он уже существует.
	 *
	 * Эта функция проверяет, существует ли файл с указанным именем в указанной директории. Если файл существует, добавляется числовой суффикс к имени файла, чтобы избежать конфликта имен.
	 *
	 * @param directory_path Путь к директории, в которой будет проверяться наличие файла.
	 * @param file_name Имя файла, к которому может быть добавлен суффикс.
	 * @return Новый путь с добавленным суффиксом, если файл существует, или исходный путь, если файл не существует.
	 */
	fs::path addSuffixIfExists(const fs::path& directory_path, const fs::path& file_name);

	/**
	 * @brief Создает путь из вектора строк.
	 *
	 * Эта функция объединяет вектор строк в один путь, используя символ разделителя пути. Каждая строка в векторе добавляется как часть пути.
	 *
	 * @param parts Вектор строк, представляющих части пути.
	 * @return Путь, собранный из частей.
	 */
	fs::path make_path(const std::vector<std::string>& parts);

	/**
	 * @brief Определяет, является ли файл изображением по его расширению.
	 *
	 * Эта функция проверяет, имеет ли файл одно из известных расширений изображений. Список поддерживаемых расширений задается в `image_extensions`.
	 *
	 * @param file_name Путь к файлу, который нужно проверить.
	 * @return `true`, если файл является изображением, `false` в противном случае.
	 */
	bool isImageFile(const fs::path& file_name);

	/**
	 * @brief Разделяет строку на части по указанному разделителю.
	 *
	 * Эта функция разделяет строку на вектор подстрок, используя заданный символ-разделитель.
	 *
	 * @param str Входная строка, которую нужно разделить.
	 * @param delimiter Символ, который используется в качестве разделителя.
	 * @return Вектор строк, полученных после разделения.
	 */
	std::vector<std::string> split(const std::string& str, char delimiter);

	/**
	 * @brief Получает последний элемент после разделения строки.
	 *
	 * Эта функция разделяет строку по заданному разделителю и возвращает последний элемент из полученных подстрок.
	 *
	 * @param str Входная строка, которую нужно разделить.
	 * @param delimiter Символ, который используется в качестве разделителя.
	 * @return Последний элемент после разделения строки, или пустая строка, если строка пустая или не содержит разделителя.
	 */
	std::string get_last_split_element(const std::string& str, char delimiter);


	template<typename... Paths>
	std::string createPath(Paths... paths) {
		std::filesystem::path combinedPath;
		((combinedPath /= paths), ...);
		return combinedPath.string();
	}

	void xorEncryptDecrypt(std::vector<uchar>& data, const std::string& key);

	// Функция для проверки валидности имени файла
	bool isValidFileName(const std::string& fileName);
}
#endif // !IMGHIDER_H
