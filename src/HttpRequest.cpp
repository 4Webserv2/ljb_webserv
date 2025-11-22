/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:32 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:36:24 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest(){}

HttpRequest::~HttpRequest(){}

void HttpRequest::parseRequestLine(std::istringstream &stream, HttpParse &req) {
	std::string line;
	if (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream first_line(line);
		first_line >> req.method >> req.uri >> req.version;

		if (req.method.empty() || req.uri.empty() || req.version.empty())
			throw std::runtime_error("Request line malformada: campos ausentes");

		if(req.method != "GET" && req.method != "POST" && req.method != "DELETE")
			throw std::runtime_error("Método HTTP não permitido: " + req.method);
	}
}

void HttpRequest::parseHeaders(std::istringstream &stream, HttpParse &req) {
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

void HttpRequest::parseBody(std::istringstream &stream, HttpParse &req) {
	std::string line, body;
	while (std::getline(stream, line)) {
		body += line + "\n";
	}
	req.body = body;
}

HttpParse HttpRequest::httpParse(const std::string &rawRequest) {
	HttpParse req;
	std::istringstream stream(rawRequest);
	parseRequestLine(stream, req);
	parseHeaders(stream, req);
	parseBody(stream, req);
	return req;
}
