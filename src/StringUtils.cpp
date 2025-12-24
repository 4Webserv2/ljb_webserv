#include "../includes/StringUtils.hpp"
#include "../includes/Logger.hpp"

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

std::string StringUtils::ostreamToString(std::string ss) {
	std::ostringstream oss;

	oss << ss;
	return (oss.str());
}

void StringUtils::errorAndCerr(const std::string &msg) {
	Logger::error(msg);
	std::cerr << msg << std::endl;
}

std::string StringUtils::size_tToString(size_t n) {
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

std::string StringUtils::extractUriWithoutQuery(const std::string &uri) {
    size_t pos = uri.find('?');
    if (pos != std::string::npos) {
        return uri.substr(0, pos);
    }
    return uri;
}

std::string StringUtils::extractQueryFromUri(const std::string &uri) {
    size_t pos = uri.find('?');
    if (pos != std::string::npos && pos + 1 < uri.size()) {
        return uri.substr(pos + 1);
    }
    return "";
}

std::string StringUtils::extractUriPathInfo(const std::string &uri, const LocationBlock &location) {
    std::string path = extractUriWithoutQuery(uri);
    size_t locUriLen = location.getUri().length();

    if (path.length() > locUriLen) {
        return path.substr(locUriLen);
    }
    return "";
}
