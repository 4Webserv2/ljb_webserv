/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:11 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/29 11:05:32 by jbergfel         ###   ########.fr       */
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
	private:
		HttpParse _par;
		void parseRequestLine(std::istringstream &stream, HttpParse &parse);
		void parseHeaders(std::istringstream &stream, HttpParse &parse);
		void parseBody(std::istringstream &stream, HttpParse &parse);

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

};
