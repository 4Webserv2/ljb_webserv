# pragma once

# include "Webserv.hpp"

typedef struct e_HttpRequest {
	std::string	method;
	std::string	uri;
	std::string	version;
	std::map<std::string, std::string> headers;
	std::string	body;
} HttpRequest;

class HttpParser {
	public:
		HttpRequest httpParser(const std::string &rawRequest);
		HttpParser();
		~HttpParser();
	private:
		void parseRequestLine(std::istringstream &stream, HttpRequest &req);
		void parseHeaders(std::istringstream &stream, HttpRequest &req);
		void parseBody(std::istringstream &stream, HttpRequest &req);
};
