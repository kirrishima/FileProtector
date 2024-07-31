#ifndef HASHING_H
#define HASHING_H

#include <string>

namespace hashing {
	/**
	 * @brief ��������� SHA-256 ��� ������.
	 *
	 * @param str ������� ������ ��� �����������.
	 * @return std::string SHA-256 ��� � ���� ����������������� ������.
	 * @throw std::runtime_error � ������ ������ ��� �������� ��� ������������� ���������, ���������� ��� ���������� ����.
	 */
	std::string sha256(const std::string& str);
}

namespace safe_hashing {
	/**
	 * @brief ��������� SHA-256 ��� ������ � ���������� ����������.
	 *
	 * @param str ������� ������ ��� �����������.
	 * @return std::string SHA-256 ��� � ���� ����������������� ������ ��� ������ ������ � ������ ������.
	 * @throw ������������ ���������� � ������� �� ����� std::cout
	 */
	std::string sha256(const std::string& str);
}

#endif // HASHING_H
