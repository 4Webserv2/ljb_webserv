# pragma once

# include "Webserv.hpp"
# include "HttpParser.hpp"

class HttpResponse{
	private:
		std::string http_version;
		int status_code;
		std::string status_message;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		HttpResponse();
		~HttpResponse();

		HttpResponse	handleGet(const HttpRequest &req);
		HttpResponse	handlePost(const HttpRequest &req);
		HttpResponse	handleDelete(const HttpRequest &req);
		HttpResponse	dispatchRequest(const HttpRequest &req);

		void			setStatus(int code, const std::string &message);
		void			setHeader(const std::string &key, const std::string &value);
		void			setBody(const std::string &b, const std::string &contentType);
		void			setErrorPage(int code);

		std::string		toString() const;
		std::string		intToString(int n) const;
};
