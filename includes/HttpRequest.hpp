# pragma once

# include "Webserv.hpp"

typedef struct e_HttpParse {
	std::string	method;
	std::string	uri;
	std::string	version;
	std::map<std::string, std::string> headers;
	std::string	body;
} HttpParse;

class HttpRequest {
	private:
		HttpParse _par;
		
		void parseHeaders(std::istringstream &stream, HttpParse &parse);
		void parseBody(std::istringstream &stream, HttpParse &parse);
		std::string unfoldHeader(const std::string &value);
		std::string decodeChunked(const std::string &chunkedBody);

	public:
		HttpParse httpParse(const std::string &rawParse);
		HttpRequest();
		~HttpRequest();

		void setPar(HttpParse parse);
		std::string getMethod() const;
		std::string getUri() const;
		std::string getVersion() const;
		std::map<std::string, std::string> getHeaders() const;
		std::string getBody() const;
		void parseRequestLine(std::istringstream &stream, HttpParse &parse);

		std::string getHeader(const std::string &name) const;
		bool hasHeader(const std::string &name) const;

};
