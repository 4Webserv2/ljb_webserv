// #pragma once

// #include <string>
// #include <algorithm>
// #include <cctype>
// #include <sstream>
// # include "LocationBlock.hpp"

// class StringUtils {
// public:
// 	static std::string toLower(const std::string &str);

// 	// Trim (remover espaços)
// 	static std::string trim(const std::string &str);
// 	static std::string ltrim(const std::string &str);
// 	static std::string rtrim(const std::string &str);

// 	// Normalizar header name (case-insensitive)
// 	static std::string normalizeHeaderName(const std::string &name);

// 	// Verificar se é espaço (incluindo tabs)
// 	static bool isWhitespace(char c);

// 	// Passa um int qualquer para uma string
// 	static std::string intToString(int n);
// 	//Passa uma string e consegue somar com outra string
// 	static std::string ostreamToString(std::string ss);
// 	//Além de usar Logger::error, joga o erro pro std::cerr
// 	static void errorAndCerr(const std::string &msg);
// 	//Passa size_t para string
// 	static std::string size_tToString(size_t n);
// 	static std::string extractUriWithoutQuery(const std::string &uri);
// 	static std::string extractQueryFromUri(const std::string &uri);
// 	static std::string extractAndDecodeUri(const std::string &uri);
// 	static std::string extractUriPathInfo(const std::string &uri, const LocationBlock &location);
// };
