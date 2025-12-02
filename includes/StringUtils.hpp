#pragma once

#include <string>
#include <algorithm>
#include <cctype>

class StringUtils {
public:
	static std::string toLower(const std::string &str);
	
	// Trim (remover espaços)
	static std::string trim(const std::string &str);
	static std::string ltrim(const std::string &str);
	static std::string rtrim(const std::string &str);
	
	// Normalizar header name (case-insensitive)
	static std::string normalizeHeaderName(const std::string &name);
	
	// Verificar se é espaço (incluindo tabs)
	static bool isWhitespace(char c);
};