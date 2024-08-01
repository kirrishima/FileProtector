#ifndef HASHING_H
#define HASHING_H

#include <string>

namespace hashing {
	/**
	 * @brief Вычисляет SHA-256 хэш строки.
	 *
	 * @param str Входная строка для хеширования.
	 * @return std::string SHA-256 хэш в виде шестнадцатеричной строки.
	 * @throw std::runtime_error В случае ошибки при создании или инициализации контекста, обновлении или завершении хэша.
	 */
	std::string sha256(const std::string& str);
}

namespace safe_hashing {
	/**
	 * @brief Вычисляет SHA-256 хэш строки с обработкой исключений.
	 *
	 * @param str Входная строка для хеширования.
	 * @return std::string SHA-256 хэш в виде шестнадцатеричной строки или пустая строка в случае ошибки.
	 * @throw обрабатывает исключения и выводит их через std::cout
	 */
	std::string sha256(const std::string& str);
}

#endif // HASHING_H
