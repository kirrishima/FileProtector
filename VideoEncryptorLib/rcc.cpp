#include "VideoEncryptor.h"
#include "pch.h"
#include <iostream> 
#include <vector>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <cctype>
#include <cwctype>

#define IS_CYRILLIC(x) ((L'à' <= (x) && (x) <= L'ÿ') || (L'À' <= (x) && (x) <= L'ß'))
#define IS_LOWER_CYRILLIC(x) (L'à' <= (x) && (x) <= L'ÿ')
#define IS_LATIN(x) ((L'a' <= (x) && (x) <= L'z') || (L'A' <= (x) && (x) <= L'Z'))
std::wstring string_to_wstring(const std::string& str) {
	// Determine the length of the resulting wide string
	size_t required_size;
	mbstowcs_s(&required_size, nullptr, 0, str.c_str(), _TRUNCATE);

	// Create a buffer to hold the wide string
	std::vector<wchar_t> buffer(required_size);

	// Convert the multibyte string to a wide string
	size_t converted_size;
	mbstowcs_s(&converted_size, buffer.data(), buffer.size(), str.c_str(), _TRUNCATE);

	// Create a std::wstring from the buffer
	std::wstring wstr(buffer.data(), converted_size - 1);  // Exclude the null terminator

	return wstr;
}

std::string wstring_to_string(const std::wstring& wstr) {
	// Determine the length of the resulting multibyte string
	size_t required_size;
	wcstombs_s(&required_size, nullptr, 0, wstr.c_str(), _TRUNCATE);

	// Create a buffer to hold the multibyte string
	std::vector<char> buffer(required_size);

	// Convert the wide string to a multibyte string
	size_t converted_size;
	wcstombs_s(&converted_size, buffer.data(), buffer.size(), wstr.c_str(), _TRUNCATE);

	// Create a std::string from the buffer
	std::string str(buffer.data(), converted_size - 1);  // Exclude the null terminator

	return str;
}

namespace RCC
{
	std::string encryptFilename(std::wstring filename, short step, short maxShift)
	{
		short shift = 0;
		for (size_t i = 0; i < filename.find_last_of('.'); i++)
		{
			if (IS_LATIN(filename[i]))
			{
				wchar_t base = (islower(filename[i])) ? L'a' : L'A';
				filename[i] = base + (filename[i] - base + shift + 26) % 26;
				shift = (shift + step) % maxShift;
			}
			else if (IS_CYRILLIC(filename[i]))
			{
				wchar_t base = (IS_LOWER_CYRILLIC(filename[i])) ? L'à' : L'À';
				filename[i] = base + (filename[i] - base + shift + 32) % 32;
				shift = (shift + step) % maxShift;
			}
		}

		return wstring_to_string(filename);
	}
}