# pragma once

# include "HttpRequest.hpp"
# include <sstream>
# include <iostream>


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
