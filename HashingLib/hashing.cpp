#include "pch.h"
#include "hashing.h"
#include "framework.h"
#include <iostream>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace hashing {
	std::string sha256(const std::string& str) {
		EVP_MD_CTX* context = EVP_MD_CTX_new();
		if (context == nullptr) {
			throw std::runtime_error("Failed to create EVP_MD_CTX");
		}

		const EVP_MD* md = EVP_sha256();
		if (EVP_DigestInit_ex(context, md, nullptr) != 1) {
			EVP_MD_CTX_free(context);
			throw std::runtime_error("Failed to initialize digest");
		}

		if (EVP_DigestUpdate(context, str.c_str(), str.size()) != 1) {
			EVP_MD_CTX_free(context);
			throw std::runtime_error("Failed to update digest");
		}

		unsigned char hash[EVP_MAX_MD_SIZE];
		unsigned int lengthOfHash = 0;
		if (EVP_DigestFinal_ex(context, hash, &lengthOfHash) != 1) {
			EVP_MD_CTX_free(context);
			throw std::runtime_error("Failed to finalize digest");
		}

		EVP_MD_CTX_free(context);

		std::stringstream ss;
		for (unsigned int i = 0; i < lengthOfHash; ++i) {
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
		}

		return ss.str();
	}
}

namespace safe_hashing {
	std::string sha256(const std::string& str) {
		try {
			std::string hash = hashing::sha256(str);
			return hash;
		}
		catch (...) {
			std::cout << "Неизвестная ошибка при вычислении хэша" << std::endl;
			return "";
		}
	}
}
