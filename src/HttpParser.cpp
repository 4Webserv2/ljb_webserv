# include "HttpParser.hpp"
# include <cstdlib>

void HttpParser::parseRequestLine(std::istringstream &stream, HttpRequest &req) {
    std::string line;
    if (std::getline(stream, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        std::istringstream first_line(line);
        first_line >> req.method >> req.uri >> req.version;
    }
}

void HttpParser::parseHeaders(std::istringstream &stream, HttpRequest &req) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line == "\r" || line == "") break;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        size_t sep = line.find(":");
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);
            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);
            req.headers[key] = value;
        }
    }
}

void HttpParser::parseBody(std::istringstream &stream, HttpRequest &req) {
    std::string line, body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    req.body = body;
}

HttpRequest HttpParser::httpParser(const std::string &rawRequest) {
    HttpRequest req;
    std::istringstream stream(rawRequest);
    parseRequestLine(stream, req);
    parseHeaders(stream, req);
    parseBody(stream, req);
    return req;
}

HttpParser::HttpParser(){}

HttpParser::~HttpParser(){}