/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:51:22 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/28 22:58:17 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"

ServerBlock::ServerBlock(ConfigFile &config) : _config(config), _maxBodySize(false, 0), _root(false, "./")
{
	this->_config.removeTokens(2); //| Remove os 2 primeiros tokens ('server' e '{')
	this->_config.verifyToken(EMPTY, "Configuração inválida: server: não foi encontrado nenhum servidor");

	while (this->_config.getTokens().size() > 0)
	{
		std::vector<std::string> tokens = this->_config.getTokens();
		if (tokens[0] == "listen")
			addListens();
		else if (tokens[0] == "server_name")
			addServerNames();
		else if (tokens[0] == "client_max_body_size")
			addMaxBodySize();
		else if (tokens[0] == "error_page")
			addErrorPages();
		else if (tokens[0] == "location")
			addLocation();
		else if (tokens[0] == "root")
			addRoot();
		else if (tokens[0] == "}")
		{
			this->_config.removeTokens(1);
			break;
		}
		else
			throw std::runtime_error("Configuração inválida: server: token inválido");
	}
	if (this->_maxBodySize.second == 0)
		throw std::runtime_error("Configuração inválida: server: client_max_body_size não pode ser zero.");
}

ServerBlock::~ServerBlock() {}

ServerBlock &ServerBlock::operator=(const ServerBlock &src)
{
	if (this != &src)
	{
		this->_serverNames = src._serverNames;
		this->_listen = src._listen;
		this->_maxBodySize = src._maxBodySize;
		this->_root = src._root;
		this->_locations = src._locations;
		this->_errorPages = src._errorPages;
	}
	return (*this);
}

bool ServerBlock::operator==(const ServerBlock &other) const
{
	return (this->_serverNames == other._serverNames &&
			this->_maxBodySize == other._maxBodySize &&
			this->_root == other._root &&
			this->_errorPages == other._errorPages);
}

std::vector<std::string> ServerBlock::getServerNames() const { return (this->_serverNames); }
std::vector<t_listen> ServerBlock::getListen() const { return (this->_listen); }
std::pair<bool, size_t> ServerBlock::getMaxBodySize() const { return (this->_maxBodySize); }
std::pair<bool, std::string> ServerBlock::getRoot() const { return (this->_root); }
std::map<int, std::string> ServerBlock::getErrorPages() const { return (this->_errorPages); }
std::map<std::string, LocationBlock> ServerBlock::getLocations() const { return (this->_locations); }

bool ServerBlock::hasListenDuplicate(const t_listen &listen) const
{
	for (size_t i = 0; i < this->_listen.size(); i++)
	{
		if (this->_listen[i].host == listen.host && this->_listen[i].port == listen.port)
			return (true);
	}
	return (false);
}

bool ServerBlock::hasListenDuplicateWith(const ServerBlock &other) const
{
	for (size_t i = 0; i < this->_listen.size(); i++)
	{
		if (other.hasListenDuplicate(this->_listen[i]))
			return (true);
	}
	return (false);
}

void ServerBlock::printServerBlock()
{
	std::cout << "Server names: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); it++)
		std::cout << *it << " " << std::endl;

	std::cout << "Max body size: " << this->_maxBodySize.second << std::endl;

	std::cout << "Root: " << this->_root.second << std::endl;

	std::cout << "Error pages: " << std::endl;
	for (std::map<int, std::string>::iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); ++it)
		std::cout << "Code: " << it->first << " | URI: " << it->second << std::endl;
}

bool ServerBlock::isUriValid(const std::string uri)
{
	std::map<std::string, LocationBlock>::iterator it = this->_locations.find(uri);
	if (it != this->_locations.end())
		return (true);
	for (it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		std::string locationPath = it->first;
		if (uri.find(locationPath) == 0)
			return (true);
	}
	return (false);
}

const LocationBlock *ServerBlock::getValidLocation(const std::string uri, const std::string method) const
{
	const LocationBlock *location = NULL;

	if (method != "GET" && method != "POST" && method != "DELETE")
		return (location);
	std::map<std::string, LocationBlock>::const_iterator it = this->_locations.find(uri);
	if (it != this->_locations.end())
	{
		std::vector<std::string> allowedMethods = it->second.getAllowMethods();
		if (allowedMethods.empty())
			return &(it->second);
		for (size_t i = 0; i < allowedMethods.size(); i++)
		{
			if (allowedMethods[i] == method)
				return &(it->second);
		}
		return (NULL);
	}
	std::string bestMatch = "";
	for (std::map<std::string, LocationBlock>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		std::string locationPath = it->first;
		if (uri.compare(0, locationPath.size(), locationPath) == 0)
		{
			if (locationPath.size() > bestMatch.size())
			{
				bool methodAllowed = false;
				bestMatch = locationPath;
				location = &(it->second);

				std::vector<std::string> allowedMethods = it->second.getAllowMethods();
				if (allowedMethods.empty())
					methodAllowed = true;
				else
				{
					for (size_t i = 0; i < allowedMethods.size(); i++)
					{
						if (allowedMethods[i] == method)
						{
							methodAllowed = true;
							break;
						}
					}
				}
				if (!methodAllowed)
				{
					bestMatch = "";
					location = NULL;
				}
			}
		}
	}

	return (location);
}

static bool isAllNumber(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
		if (!isdigit(s[i]))
			return (false);
	return (true);
}

static unsigned int strToIpv4(std::string s)
{
	if (s == "localhost")
		s = "127.0.0.1";

	std::istringstream ss(s);
	std::string octet_str;
	std::vector<unsigned int> octets;

	while (std::getline(ss, octet_str, '.'))
	{
		if (octet_str.empty() || isAllNumber(octet_str) == false)
			throw std::runtime_error("Configuração inválida: listen: host: o octeto é inválido, contém caracteres não numéricos");

		unsigned int octet = std::atoi(octet_str.c_str());
		if (octet > 255)
			throw std::runtime_error("Configuração inválida: listen: host: o octeto é inválido, é maior que 255");
		octets.push_back(octet);
	}

	if (octets.size() != 4) //| Verificando se tem mais de 4 octetos
		throw std::runtime_error("Configuração inválida: listen: host: o host inválido, possui mais de 4 octetos");
	return ((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
}

void ServerBlock::addListens()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(EMPTY, "Configuração inválida: listen: não foi encontrado nenhum listen");

	std::string host_port = this->_config.getTokens()[0];
	if (host_port == ";")
		host_port = "0.0.0.0:80";

	std::string before;
	if (host_port.find(':') != std::string::npos)
		before = host_port.substr(0, host_port.find(':'));
	else
		before = "0.0.0.0";
	unsigned int host = strToIpv4(before);
	std::string after = host_port.substr(host_port.find(':') + 1, host_port.length() - host_port.find(':'));
	int port;
	if (after == before)
		port = 80;
	else
	{
		if (isAllNumber(after) == false)
			throw std::runtime_error("Configuração inválida: listen: port: é inválido, deve ser um número");
		port = std::atoi(after.c_str());
		if (port < 1 || port > 65535)
			throw std::runtime_error("Configuração inválida: listen: port: é inválido, deve ser um número entre 1 e 65535");
	}

	t_listen listen;
	listen.host = host;
	listen.port = port;
	if (this->hasListenDuplicate(listen))
		throw std::runtime_error("Configuração inválida: listen: listen duplicado no mesmo server block");

	this->_listen.push_back(listen);
	if (this->_config.getTokens()[0] != ";")
		this->_config.removeTokens(1);
	this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: listen: esperava um ponto e vírgula no final de listen");
	this->_config.removeTokens(1);
}

void ServerBlock::addServerNames()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Configuração inválida: server_name: não foi encontrado nenhum server_name");

	std::vector<std::string> names;
	while (this->_config.getTokens()[0] != ";")
	{
		this->_config.verifyToken(END_OF_FILE, "Configuração inválida: server_name: final do arquivo encontrado");
		names.push_back(this->_config.getTokens()[0]);
		this->_config.removeTokens(1);
	}

	for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it)
		this->_serverNames.push_back(*it);

	for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); ++it)
	{
		for (std::vector<std::string>::iterator jt = it + 1; jt != this->_serverNames.end();)
		{
			if (*it == *jt)
				jt = this->_serverNames.erase(jt);
			else
				++jt;
		}
	}

	this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: server_name: esperava um ponto e vírgula no final de server_name");
	this->_config.removeTokens(1);
}

void ServerBlock::addMaxBodySize()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Configuração inválida: client_max_body_size: não foi encontrado nenhum client_max_body_size");

	if (this->_maxBodySize.first == true)
		throw std::runtime_error("Configuração inválida: client_max_body_size: client_max_body_size já foi definido");
	this->_maxBodySize.first = true;

	std::string value = this->_config.getTokens()[0];
	size_t i = 0;
	while (i < value.size() - 1)
	{
		if (!isdigit(value[i]))
			throw std::runtime_error("Configuração inválida: client_max_body_size: é inválido, deve ser um número");
		i++;
	}

	this->_maxBodySize.second = std::atoi(value.c_str());
	if (value[i] == 'G')
		this->_maxBodySize.second *= 1024 * 1024 * 1024;
	else if (value[i] == 'M')
		this->_maxBodySize.second *= 1024 * 1024;
	else if (value[i] == 'K')
		this->_maxBodySize.second *= 1024;
	else if (value[i] == 'B')
		this->_maxBodySize.second *= 1;
	else
		throw std::runtime_error("Configuração inválida: client_max_body_size: é inválido, deve ser um número seguido de unidade");

	this->_config.removeTokens(1);
	this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: client_max_body_size: esperava um ponto e vírgula no final de client_max_body_size");
	this->_config.removeTokens(1);
}

void ServerBlock::addErrorPages()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Configuração inválida: error_page: não foi encontrado nenhum error_page");
	std::vector<std::string> codes_str;
	while (this->_config.getTokens()[0] != ";")
	{
		this->_config.verifyToken(END_OF_FILE, "Configuração inválida: error_page: final do arquivo encontrado");
		codes_str.push_back(this->_config.getTokens()[0]);
		this->_config.removeTokens(1);
	}

	std::string uri = codes_str.back();
	codes_str.pop_back();

	std::vector<int> codes;
	for (std::vector<std::string>::iterator it = codes_str.begin(); it != codes_str.end(); ++it)
	{
		if (isAllNumber(*it) == false)
			throw std::runtime_error("Configuração inválida: error_page: [code] é inválido, deve ser um número");

		int code = std::atoi(it->c_str());
		if (code < 100 || code > 599)
			throw std::runtime_error("Configuração inválida: error_page: [code] é inválido, deve ser um número entre 100 e 599");

		codes.push_back(code);
	}

	for (std::vector<int>::iterator it = codes.begin(); it != codes.end(); ++it)
		this->_errorPages[*it] = uri;

	this->_config.verifyToken(EMPTY, "Configuração inválida: error_page: esperava um ponto e vírgula no final de error_page");
	this->_config.removeTokens(1);
}

void ServerBlock::addLocation()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(EMPTY, "Configuração inválida: location: não foi encontrado nenhum location");

	std::string uri = this->_config.getTokens()[0];
	if (uri[0] != '/')
		throw std::runtime_error("Configuração inválida: location: URI inválida, deve começar com '/'");

	if (this->_locations.count(uri) > 0)
		throw std::runtime_error("Configuração inválida: location: location duplicado");
	this->_locations.insert(std::make_pair(uri, LocationBlock(this->_config)));

	this->_config.verifyToken(EMPTY, "Configuração inválida: location: esperava um ponto e vírgula no final de location");
}

void ServerBlock::addRoot()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Configuração inválida: root: não foi encontrado nenhuma root");

	this->_config.verifyToken(END_OF_FILE, "Configuração inválida: root: final do arquivo encontrado");

	if (this->_root.first == true)
		throw std::runtime_error("Configuração inválida: root: root já foi definido");
	this->_root.first = true;

	this->_root.second = this->_config.getTokens()[0];

	this->_config.removeTokens(1);
	this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: root: esperava um ponto e vírgula no final de root");
	this->_config.removeTokens(1);
