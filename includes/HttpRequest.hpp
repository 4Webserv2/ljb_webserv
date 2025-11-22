/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:11 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:36:10 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	public:
		HttpParse httpParse(const std::string &rawParse);
		HttpRequest();
		~HttpRequest();
	private:
		void parseRequestLine(std::istringstream &stream, HttpParse &req);
		void parseHeaders(std::istringstream &stream, HttpParse &req);
		void parseBody(std::istringstream &stream, HttpParse &req);
};
