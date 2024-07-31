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
	*@brief ������� ����������� �� �����
	*
	*@param image ������: cv::Mat, �������������� �����������
	*@param displayName: ��������� ���� � ������������
	*@return ���������� ���������� ���������� ����
	**/
	bool displayImage(const cv::Mat& image, const std::string& displayName);

	/**
	 * @brief ������� ����������� � ������� �� �����, ���� ��� ������������
	 *
	 * @param binaryPath: ����, � ������� ����� ����������� �����
	 * @param searchFilename: �������� ����������� ��� ������
	 * @return ���������, ���� �� ������� �����������
	 */
	bool findAndDisplayImage(const std::string& binaryPath, const std::string& searchFilename);
	/**
	 * @brief ��������� ��� �������� ����������.
	 *
	 * @details ������ ��������� ������������ ��� �������� ����� � ����� verbose,
	 * ������� ����� ���� ����������� ��� ������ �������������� ����������.
	 */
	struct Params {
		std::vector<std::string> paths; ///< ������ �����, ���������� ����.
		bool verbose = false;           ///< ���� ��� ���������/���������� ���������� ������.
	};
	/**
	 * @brief ��������� ����������� � ��� ���������� � �������� ����.
	 *
	 * @param binaryPath ���� � ��������� �����, � ������� ����� ��������� �����������.
	 * @param imageName ��� �����������, ������� ����� ���������.
	 * @param image �����������, ������� ����� ���������.
	 *
	 *@return ���������� ����� ����������
	 */
	bool saveImage(const std::string& binaryPath, const std::string& imageName, const cv::Mat& image);

	/**
	* @brief ���������� ����������� � .bin ����
	*
	* @param directoryPath: ����������, �� ������� ����� ������� �����������, ����� ������ - ���������
	* @param binaryPath: ���� � ��������� �����
	* @param hashFilePath: ���� � ����� � ����� �������� �����������
	*/
	void saveImagesToBinary(const std::string& directoryPath, const std::string& binaryPath, const std::string& hashFilePath);
	/**
	 * @brief ��������� ����������� � ��� ���������� �� ��������� �����.
	 *
	 * @param binaryPath: ���� � ��������� �����, �� �������� ����� ����������� �����������.
	 * @param start: ��������� ������� � �������� �����, � ������� �������� ������.
	 * @return std::tuple<cv::Mat, std::string, int, bool> ������, ���������� �����������,
	 * �������� ����������� + ����������, ������� � ����� ����� ������ � ���� ������.
	 */
	std::tuple<cv::Mat, std::string, size_t, bool> loadImage(const std::string& binaryPath, const size_t start);

	/**
	 * @brief ��������� ����������� � �� ���������� �� ��������� ����� � ��������� �� � ��������� ����������.
	 *
	 * @param binaryPath: ���� � ��������� �����, �� �������� ����� ��������� �����������.
	 * @param outputDirectory: ����������, � ������� ����� ��������� ����������� �����������.
	 */
	void loadImagesFromBinary(const std::string& binaryPath, const std::string& outputDirectory);

	/**
	 * @brief ������� ����� �� �������� ����������
	 *
	 * @param directoryPath: ����������, ������� ���� �������.
	 */
	void clearDirectory(const std::string& directoryPath, bool deleteSubfolders = false, bool clearSubfolders = false);
	/**
	 * @brief ���������� true, ���� ���������� ���� �������� ������
	 *
	 * @param path: ���� ��� ��������
	 */

	 //bool isDirectory(const std::string& path);

	 /**
	  * @brief ������� ��������� ���������� � �����, ���� �� �� ����������
	  *
	  * @param params: ��������� � �����������, � ������ paths (������ �����, ���������� ����) � verbose (���� ��� ���������/���������� ���������� ������)
	  * @return ���������, ���� �� ������� �����-���� ���������� ��� �����
	  */
	bool checkAndCreatePaths(const Params& params);

	/**
	 * @brief  ���������, ���������� �� ��� ��� �������� ����������� � �����
	 *
	 * @param hashFilePath: ���� � ����� � �����
	 * @param fileHash: ��� ��� ������
	 * @return ���������� ��������� ������
	 */
	bool isNameHashInFile(const std::string& hashFilePath, const std::string& fileHash);

	/**
	 * @brief ���������� ��� � ���� � �����
	 *
	 * @param hashFilePath: ���� � ����� � ������
	 * @param hashValue: �������� ����
	 */
	void addNameHashToFile(const std::string& hashFilePath, const std::string& hashValue);

	// ������� ��� ��������� ����������� ����� ����� � ����������� ������� ���� � �������
	/**
	 * @brief ���������� ���������� ��� ����� � ����������� ������� ���� � �������
	 *
	 * @param originalFilename: �������� ����� ��� ���������, � �����������
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