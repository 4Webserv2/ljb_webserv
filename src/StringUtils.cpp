#include "../includes/StringUtils.hpp"

std::string StringUtils::toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool StringUtils::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::string StringUtils::ltrim(const std::string &str) {
    size_t start = 0;
    while (start < str.length() && isWhitespace(str[start])) {
        start++;
    }
    return str.substr(start);
}

std::string StringUtils::rtrim(const std::string &str) {
    size_t end = str.length();
    while (end > 0 && isWhitespace(str[end - 1])) {
        end--;
    }
    return str.substr(0, end);
}

std::string StringUtils::trim(const std::string &str) {
    return ltrim(rtrim(str));
}

std::string StringUtils::normalizeHeaderName(const std::string &name) {
    // Headers HTTP são case-insensitive
    // Convertemos para minúsculas para comparação
    return toLower(trim(name));
}

std::string StringUtils::intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

std::string ostreamToString(std::string ss) {
	std::ostringstream oss;

	oss << ss;
	return (oss.str());
}

