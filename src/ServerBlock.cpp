/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:59 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/23 20:00:28 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerBlock.hpp"

ServerBlock::ServerBlock(ServerConfig &config)
	: _config(config),
	  _maxBodySize(std::make_pair(false, 0)),
	  _root(std::make_pair(false, "")),
	  _cgiTimeout(30)  // ADICIONAR: Timeout padrão de 30 segundos
{
	this->_config.removeTokens(2); //| Remove os 2 primeiros tokens ('server' e '{')
	this->_config.verifyToken(EMPTY, "Invalid configuration: server: no server block found");

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
			throw std::runtime_error("Invalid configuration: server: invalid token");
	}
/*	for (std::map<std::string, LocationBlock>::iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		std::cout << "Location: " << it->first << " with methods: ";
		std::vector<std::string> methods = it->second.getAllowMethods();
		for (size_t i = 0; i < methods.size(); i++)
			std::cout << methods[i] << " ";
		std::cout << std::endl;
	}*/
	std::cout << "SAIU FOR" << std::endl;
	//| Fazer verificação para ver se os atributos estão corretos.
	if (this->_maxBodySize.second == 0)
		throw std::runtime_error("Invalid configuration: server: client_max_body_size cannot be zero.");
}

ServerBlock::ServerBlock(const ServerBlock &src)
	: _config(src._config),  // Referências devem ser inicializadas na lista de inicialização
	_serverNames(src._serverNames),
	_listen(src._listen),
	_maxBodySize(src._maxBodySize),
	_root(src._root),
	_locations(src._locations),
	_errorPages(src._errorPages)
{
}

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
	return *this;
}

ServerBlock::~ServerBlock() {
	std::cout << "Server Block destructor" << std::endl;
}

const std::vector<std::string>& ServerBlock::getServerNames() const { return this->_serverNames; }

const std::vector<t_listen>& ServerBlock::getListen() const { return this->_listen; }

const std::pair<bool, size_t>& ServerBlock::getMaxBodySize() const { return this->_maxBodySize; }

const std::pair<bool, std::string>& ServerBlock::getRoot() const { return this->_root; }

const std::map<int, std::string>& ServerBlock::getErrorPages() const { return this->_errorPages; }

const std::map<std::string, LocationBlock>& ServerBlock::getLocations() const { return this->_locations; }

void ServerBlock::printServerBlock()
{
	std::cout << "Server names: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); it++)
		std::cout << *it << " " << std::endl;

	std::cout << "Max body size: " << this->_maxBodySize.second << std::endl;

	std::cout << "Root: " << this->_root.second << std::endl;

	std::cout << "Listens: " << std::endl;
	for (size_t i = 0; i < this->_listen.size(); i++)
		std::cout << "Host[" << i << "]: " << this->_listen[i].host << " Port[" << i << "]: " << this->_listen[i].port << std::endl;

	std::cout << "Error pages: " << std::endl;
	for (std::map<int, std::string>::iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); ++it)
		std::cout << "Code: " << it->first << " | URI: " << it->second << std::endl;
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
			throw std::runtime_error("Invalid configuration: listen: host: the octet is invalid, contains non-numeric characters");

		unsigned int octet = std::atoi(octet_str.c_str());
		if (octet > 255)
			throw std::runtime_error("Invalid configuration: listen: host: the octet is invalid, greater than 255");

		octets.push_back(octet);
	}

	if (octets.size() != 4) //| Verificando se tem mais de 4 octetos
		throw std::runtime_error("Invalid configuration: listen: host: invalid host, has more than 4 octets");

	return ((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
}

bool ServerBlock::isUriValid(const std::string uri)
{
	// Verificar se URI exato existe
	std::map<std::string, LocationBlock>::iterator it = this->_locations.find(uri);
	if (it != this->_locations.end())
		return (true);

	// Verificar se URI começa com alguma location válida
	// Ex: /test.css deve casar com location /
	for (it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		std::string locationPath = it->first;
		// Verificar se URI começa com este location path
		if (uri.find(locationPath) == 0)
		{
			return (true);
		}
	}

	return (false);
}

bool ServerBlock::isLocationValid(const std::string uri, const std::string method)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		return (false);

	// Verificar se URI exato existe
	std::map<std::string, LocationBlock>::iterator it = this->_locations.find(uri);
	if (it != this->_locations.end())
	{
		std::vector<std::string> allowedMethods = it->second.getAllowMethods();
		if (allowedMethods.empty())
			return (true);
		for (size_t i = 0; i < allowedMethods.size(); i++)
		{
			if (allowedMethods[i] == method)
				return (true);
		}
		return (false);
	}

	// Verificar se URI começa com alguma location válida
	// Ex: /test.css deve casar com location /
	for (it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		std::string locationPath = it->first;
		// Verificar se URI começa com este location path
		if (uri.find(locationPath) == 0)
		{
			std::vector<std::string> allowedMethods = it->second.getAllowMethods();
			if (allowedMethods.empty())
				return (true);
			for (size_t i = 0; i < allowedMethods.size(); i++)
			{
				if (allowedMethods[i] == method)
					return (true);
			}
			return (false);
		}
	}

	return (false);
}

void ServerBlock::addListens()
{
	this->_config.removeTokens(1); //| Removendo o token 'listen'
	this->_config.verifyToken(EMPTY, "Invalid configuration: listen: no listen directive found");

	std::string host_port = this->_config.getTokens()[0];
	if (host_port == ";") //| Caso não seja especificado nenhum Host e Port, tem a padrão 0.0.0.0:80
		host_port = "0.0.0.0:80";

	//| Pegar conteúdo que vem antes do : e transformar em Host
	std::string before;
	if (host_port.find(':') != std::string::npos)
		before = host_port.substr(0, host_port.find(':'));
	else
		before = "0.0.0.0";
	unsigned int host = strToIpv4(before);

	//| Pegar conteúdo que vem depois do : e transformar em Port
	//| Para o Port, tem que verificar antes se tem o : se não vai duplicar o Host > listen localhost -> Host: localhost & Port: localhost
	std::string after = host_port.substr(host_port.find(':') + 1, host_port.length() - host_port.find(':'));
	int port;
	if (after == before) //| Caso a porta não seja especificada
		port = 80;
	else
	{
		if (isAllNumber(after) == false)
			throw std::runtime_error("Invalid configuration: listen: port: invalid, must be a number");

		port = std::atoi(after.c_str());
		if (port < 1 || port > 65535)
			throw std::runtime_error("Invalid configuration: listen: port: invalid, must be a number between 1 and 65535");
	}

	t_listen listen;
	listen.host = host;
	listen.port = port;

	//| Remover duplicatas de listen (?)
	for (size_t i = 0; i < this->_listen.size(); i++)
		if (this->_listen[i].host == listen.host && this->_listen[i].port == listen.port)
			throw std::runtime_error("Invalid configuration: listen: duplicate listen"); //| Ou somente remover duplicatas?

	this->_listen.push_back(listen);

	if (this->_config.getTokens()[0] != ";")
		this->_config.removeTokens(1); //| Removendo o argumento de listen
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: listen: expected a semicolon at the end of listen");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}
void ServerBlock::addServerNames()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: server_name: no server_name found");

	std::vector<std::string> names;
	while (this->_config.getTokens()[0] != ";")
	{
		this->_config.verifyToken(END_OF_FILE, "Invalid configuration: server_name: end of file reached");
		names.push_back(this->_config.getTokens()[0]);
		this->_config.removeTokens(1);
	}

	for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it)
		this->_serverNames.push_back(*it);

	for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end();)
	{
		std::vector<std::string>::iterator jt = it + 1;
		while (jt != this->_serverNames.end())
		{
			if (*it == *jt)
				jt = this->_serverNames.erase(jt);
			else
				++jt;
		}
		++it;
	}

	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: server_name: expected a semicolon at the end of server_name");
	this->_config.removeTokens(1);
}

void ServerBlock::addMaxBodySize()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: client_max_body_size: no client_max_body_size found");

	if (this->_maxBodySize.first == true)
		throw std::runtime_error("Invalid configuration: client_max_body_size: client_max_body_size already defined");
	this->_maxBodySize.first = true;

	std::string value = this->_config.getTokens()[0];
	size_t i = 0;
	while (i < value.size() - 1)
	{
		if (!isdigit(value[i]))
			throw std::runtime_error("Invalid configuration: client_max_body_size: invalid, must be a number");
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
		throw std::runtime_error("Invalid configuration: client_max_body_size: invalid, must be a number followed by a unit");

	this->_config.removeTokens(1);
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: client_max_body_size: expected a semicolon at the end of client_max_body_size");
	this->_config.removeTokens(1);
}

void ServerBlock::addErrorPages()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: error_page: no error_page found");

	std::vector<std::string> codes_str;
	while (this->_config.getTokens()[0] != ";")
	{
		this->_config.verifyToken(END_OF_FILE, "Invalid configuration: error_page: end of file reached");
		codes_str.push_back(this->_config.getTokens()[0]);
		this->_config.removeTokens(1);
	}

	std::string uri = codes_str.back();
	codes_str.pop_back();

	std::vector<int> codes;
	for (std::vector<std::string>::iterator it = codes_str.begin(); it != codes_str.end(); ++it)
	{
		if (isAllNumber(*it) == false)
			throw std::runtime_error("Invalid configuration: error_page: [code] invalid, must be a number");

		int code = std::atoi(it->c_str());
		if (code < 100 || code > 599)
			throw std::runtime_error("Invalid configuration: error_page: [code] invalid, must be a number between 100 and 599");

		codes.push_back(code);
	}

	for (std::vector<int>::iterator it = codes.begin(); it != codes.end(); ++it)
		this->_errorPages[*it] = uri;

	this->_config.verifyToken(EMPTY, "Invalid configuration: error_page: expected a semicolon at the end of error_page");
	this->_config.removeTokens(1);
}

void ServerBlock::addLocation()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(EMPTY, "Invalid configuration: location: no location found");

	std::string uri = this->_config.getTokens()[0];
	if (uri[0] != '/')
		throw std::runtime_error("Invalid configuration: location: invalid URI, must start with '/'");

	for (std::map<std::string, LocationBlock>::iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
		std::cout << "Existing location: " << it->first << std::endl;
	if (this->_locations.count(uri) > 0)
		throw std::runtime_error("Invalid configuration: location: duplicate location");
	this->_locations.insert(std::make_pair(uri, LocationBlock(this->_config)));

	this->_config.verifyToken(EMPTY, "Invalid configuration: location: expected a semicolon at the end of location");
}

void ServerBlock::addRoot()
{
	this->_config.removeTokens(1);
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: root: no root found");

	this->_config.verifyToken(END_OF_FILE, "Invalid configuration: root: end of file reached");

	if (this->_root.first == true)
		throw std::runtime_error("Invalid configuration: root: root already defined");
	this->_root.first = true;

	this->_root.second = this->_config.getTokens()[0];

	this->_config.removeTokens(1);
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: root: expected a semicolon at the end of root");
	this->_config.removeTokens(1);
}

unsigned int ServerBlock::getCgiTimeout() const {
	return this->_cgiTimeout;
}

void ServerBlock::setCgiTimeout(unsigned int timeout) {
	this->_cgiTimeout = timeout;
}
