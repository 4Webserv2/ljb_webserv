/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:08 by jbergfel          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2025/12/18 20:06:13 by lraggio          ###   ########.fr       */
=======
/*   Updated: 2025/12/23 21:42:23 by btaveira         ###   ########.fr       */
>>>>>>> fix/segfault
/*                                                                            */
/* ************************************************************************** */

# pragma once

# include "Webserv.hpp"
# include "HttpRequest.hpp"
# include "LocationBlock.hpp"
# include "StringUtils.hpp"
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <dirent.h>
//# include "Client.hpp"
# include <ctime>

// Forward declaration
class Client;
class LocationBlock;
class ServerBlock;
class LocationBlock;

class HttpResponse {
<<<<<<< HEAD
    private:
        std::string http_version;
        std::string status_message;
        std::map<std::string, std::string> headers;
        std::string body;

        const std::map<int, std::string>* _errorPages;
        std::string _rootPath;

        // Métodos auxiliares para upload
        HttpResponse handleSimpleUpload(const HttpRequest &req,
                                       const std::string &uploadDir,
                                       HttpResponse &res);
        HttpResponse handleMultipartUpload(const HttpRequest &req,
                                          const std::string &uploadDir,
                                          HttpResponse &res);
        std::string extractFilename(const std::string &headers);

    public:
        HttpResponse();
        ~HttpResponse();
        int status_code;

        HttpResponse handleGet(const HttpRequest &req, const ServerBlock &server, const LocationBlock &location);
        HttpResponse handlePost(const HttpRequest &req);
        HttpResponse handleDelete(const HttpRequest &req);
        HttpResponse handleCGI(const HttpRequest &req);
        HttpResponse dispatchRequest(const HttpRequest &req);

        void setStatus(int code, const std::string &message);
        void setHeader(const std::string &key, const std::string &value);
        void setBody(const std::string &b, const std::string &contentType);

        void setErrorPage(int code);
        void setErrorPageConfig(const std::map<int, std::string> *errorPages, const std::string &rootPath);

        std::string getDefaultStatusMessage(int code);
        std::string toString() const;
        std::string intToString(int n) const;
		static std::string getContentTypeFromPath(const std::string &path);
};
=======
	private:
		std::string													_http_version;
		int 																_status_code;
		std::string 												_status_message;
		std::map<std::string, std::string>	_headers;
		std::string 												_body;
		bool																_execAutoIndex;
	public:
		HttpResponse();
		~HttpResponse();

	void generateAutoIndexHTML(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handleGet(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handlePost(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handleDelete(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void dispatchRequest(Client *client, const ServerBlock &serverBlock, const LocationBlock &location);

	void setStatus(int code, const std::string &message);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &b, const std::string &contentType);
	void setErrorPage(int code, const ServerBlock *serverBlock = NULL);
	void setResponseByStatus(int statusCode, const ServerBlock *serverBlock = NULL, const std::string &bodyContent="", const std::string &contentType="text/html");
	void processCookies(const HttpRequest &req, const LocationBlock &location);
	void parseCgiOutput(const std::string& cgiRawOutput); // Adicionado

	std::string		toString() const;
	std::string		intToString(int n) const;

	std::string		getMimeType(const std::string &path) const;
	std::string		getStatusMessageForCode(int code) const;
	std::string 	getHttpVersion() const;
	void setExecAutoIndex(bool exec);
	
	std::string findBestLocationMatch(const std::string &uri, const ServerBlock &serverBlock, LocationBlock &location) const;

	bool 			getExecAutoIndex() const;
	
	int getStatusCode() const;
	std::string getStatusMessage() const;
	std::string getHeaderValue(const std::string &key) const;
};
>>>>>>> fix/segfault
