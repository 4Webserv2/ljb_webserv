/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:08 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:21:08 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# pragma once

# include "Webserv.hpp"
# include "HttpRequest.hpp"
# include <sys/stat.h>
# include <sys/types.h>
# include <ctime>

class HttpResponse{
	private:
		std::string http_version;
		std::string status_message;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		HttpResponse();
		~HttpResponse();
		int status_code;
		HttpResponse	handleGet(const HttpParse &req);
		HttpResponse	handlePost(const HttpParse &req);
		HttpResponse	handleDelete(const HttpParse &req);
		HttpResponse	handleCGI(const HttpParse &req);
		HttpResponse	dispatchRequest(const HttpParse &req);

		void			setStatus(int code, const std::string &message);
		void			setHeader(const std::string &key, const std::string &value);
		void			setBody(const std::string &b, const std::string &contentType);
		void			setErrorPage(int code);

		std::string		toString() const;
		std::string		intToString(int n) const;
};
