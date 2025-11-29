/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:32 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/29 11:05:49 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest(){}

HttpRequest::~HttpRequest(){}

void HttpRequest::parseRequestLine(std::istringstream &stream, HttpParse &parse) {
	std::string line;
	if (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream first_line(line);
		first_line >> parse.method >> parse.uri >> parse.version;

		if (parse.method.empty() || parse.uri.empty() || parse.version.empty())
			throw std::runtime_error("Request line malformada: campos ausentes");

		if(parse.method != "GET" && parse.method != "POST" && parse.method != "DELETE")
			throw std::runtime_error("Método HTTP não permitido: " + parse.method);
	}
}

void HttpRequest::parseHeaders(std::istringstream &stream, HttpParse &parse) {
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
			parse.headers[key] = value;
		}
	}
}

void HttpRequest::parseBody(std::istringstream &stream, HttpParse &parse) {
	std::string line, body;
	while (std::getline(stream, line)) {
		body += line + "\n";
	}
	parse.body = body;
}

HttpParse HttpRequest::httpParse(const std::string &rawRequest) {
	HttpParse parse;
	std::istringstream stream(rawRequest);
	parseRequestLine(stream, parse);
	parseHeaders(stream, parse);
	parseBody(stream, parse);
	return parse;
}

void HttpRequest::setPar(HttpParse parse)
{
	this->_par = parse;
}

std::string HttpRequest::getMethod() const
{
	return (this->_par.method);
}

std::string HttpRequest::getUri() const
{
	return (this->_par.uri);
}

std::string HttpRequest::getVersion() const
{
	return (this->_par.version);
}

std::map<std::string, std::string> HttpRequest::getHeaders() const
{
	return (this->_par.headers);
}

std::string HttpRequest::getBody() const
{
	return (this->_par.body);
}
