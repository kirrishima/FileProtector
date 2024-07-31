#include <vector>
#include <string>
#include <tuple>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>

namespace imghider {

	static const short RCC_Shift = 3;

	/**
	*@brief Выводит изображение на экран
	*
	*@param image объект: cv::Mat, представляющий изображение
	*@param displayName: Заголовок окна с изображением
	*@return Возвращает успешность выполнения кода
	**/
	bool displayImage(const cv::Mat& image, const std::string& displayName);

	/**
	 * @brief Находит изображение и выводит на экран, если оно присутствует
	 *
	 * @param binaryPath: файл, в котором будет происходить поиск
	 * @param searchFilename: название изображения для поиска
	 * @return Указывает, было ли найдено изображение
	 */
	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename);
	/**
	 * @brief Структура для хранения параметров.
	 *
	 * @details Данная структура используется для хранения путей и флага verbose,
	 * который может быть использован для вывода дополнительной информации.
	 */
	struct Params {
		std::vector<std::string> paths; ///< Вектор строк, содержащий пути.
		bool verbose = false;           ///< Флаг для включения/выключения подробного вывода.
	};
	/**
	 * @brief Сохраняет изображение и его метаданные в бинарный файл.
	 *
	 * @param binaryPath Путь к бинарному файлу, в который будет сохранено изображение.
	 * @param imageName Имя изображения, которое будет сохранено.
	 * @param image Изображение, которое будет сохранено.
	 *
	 *@return Возвращает успех сохранения
	 */
	bool saveImage(const std::string& binaryPath, const std::string& imageName, const cv::Mat& image);

	/**
	* @brief Записывает изображения в .bin файл
	*
	* @param directoryPath: директория, из которой будут браться изображения, после записи - удаляться
	* @param binaryPath: путь к бинарному файлу
	* @param hashFilePath: путь к файлу с хэшем названий изображений
	*/
	void saveImagesToBinary(const std::string& directoryPath, const std::string& binaryPath, const std::string& hashFilePath);
	/**
	 * @brief Загружает изображение и его метаданные из бинарного файла.
	 *
	 * @param binaryPath: Путь к бинарному файлу, из которого будет загружаться изображение.
	 * @param start: Начальная позиция в бинарном файле, с которой начнется чтение.
	 * @return std::tuple<cv::Mat, std::string, int, bool> Кортеж, содержащий изображение,
	 * название изображения + расширение, позицию в файле после чтения и флаг успеха.
	 */
	std::tuple<cv::Mat, std::string, size_t, bool> loadImage(const std::string& binaryPath, const size_t start);

	/**
	 * @brief Загружает изображения и их метаданные из бинарного файла и сохраняет их в указанной директории.
	 *
	 * @param binaryPath: Путь к бинарному файлу, из которого будут загружены изображения.
	 * @param outputDirectory: Директория, в которую будут сохранены загруженные изображения.
	 */
	void loadImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory);

	/**
	 * @brief Удаляет файлы из заданной директории
	 *
	 * @param directoryPath: Директория, которую надо удалить.
	 */
	void clearDirectory(const std::string& directoryPath, bool deleteSubfolders = false, bool clearSubfolders = false);
	/**
	 * @brief Возвращает true, если переданный путь является папкой
	 *
	 * @param path: путь для проверки
	 */

	 //bool isDirectory(const std::string& path);

	 /**
	  * @brief Создает указанные директории и файлы, если их не существует
	  *
	  * @param params: структура с параметрами, с полями paths (вектор строк, содержащий пути) и verbose (Флаг для включения/выключения подробного вывода)
	  * @return указывает, были ли созданы какие-либо директории или файлы
	  */
	bool checkAndCreatePaths(const Params& params);

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

	bool resolveDuplicate(
		const std::string& directoryPath,
		const std::string& binaryPath,
		const std::string& imagePath,
		const std::string& filename,
		const std::string& fileRelPath,
		std::string& newFilename,
		std::string& fileHash,
		const cv::Mat& image
	);

	std::string get_last_split_element(const std::string& str, char delimiter);

	std::vector<std::string> split(const std::string& str, char delimiter);

	template<typename... Paths>
	std::string createPath(Paths... paths) {
		std::filesystem::path combinedPath;
		((combinedPath /= paths), ...);
		return combinedPath.string();
	}
}