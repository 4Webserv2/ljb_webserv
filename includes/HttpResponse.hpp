/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:08 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/05 08:58:30 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# pragma once

# include "Webserv.hpp"
# include "HttpRequest.hpp"
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <ctime>

// Forward declaration
class ServerBlock;

class HttpResponse{
    private:
        std::string http_version;
        std::string status_message;
        std::map<std::string, std::string> headers;
        std::string body;
        
        // Referência para error_pages do ServerBlock
        const std::map<int, std::string>* _errorPages;
        std::string _rootPath;

    public:
        HttpResponse();
        ~HttpResponse();
        int status_code;
        
        HttpResponse handleGet(const HttpRequest &req);
        HttpResponse handlePost(const HttpRequest &req);
        HttpResponse handleDelete(const HttpRequest &req);
        HttpResponse handleCGI(const HttpRequest &req);
        HttpResponse dispatchRequest(const HttpRequest &req);

        void setStatus(int code, const std::string &message);
        void setHeader(const std::string &key, const std::string &value);
        void setBody(const std::string &b, const std::string &contentType);
        
        // Métodos de erro melhorados
        void setErrorPage(int code);
        void setErrorPageConfig(const std::map<int, std::string> *errorPages, const std::string &rootPath);

        std::string toString() const;
        std::string intToString(int n) const;
		std::string	getDefaultStatusMessage(int code);
};
