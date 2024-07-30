#include "pch.h"
#include "VideoEncryptor.h"

namespace RCC
{
	std::string encryptFilename(const std::string& filename, short step, short maxShift)
	{
		short shift = 0;
		std::string result = filename;
		for (size_t i = 0; i < filename.find_last_of('.'); i++)
		{
			if (isalpha(filename[i]))
			{
				char base = (islower(filename[i])) ? 'a' : 'A';
				result[i] = base + (filename[i] - base + shift + 26) % 26;
				shift = (shift + step) % maxShift;
			}
			else if (IS_CYRILLIC(filename[i]))
			{
				char base = (IS_LOWER_CYRILLIC(filename[i])) ? 'à' : 'À';
				result[i] = base + (filename[i] - base + shift + 33) % 33;
				shift = (shift + step) % maxShift;
			}
		}
		return result;
	}
}
