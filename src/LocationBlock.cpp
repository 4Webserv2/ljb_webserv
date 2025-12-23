/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:40 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/23 18:57:42 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/LocationBlock.hpp"

LocationBlock::LocationBlock(ServerConfig &config) : _config(config), _autoIndex(false), _canUpload(false), _uploadPath("./") {
	this->_uri = this->_config.getTokens()[0];

	this->_config.removeTokens(2); //| Remove o token de URI e '{'
	this->_config.verifyToken(EMPTY, "Invalid configuration: no location block was found");

	while (this->_config.getTokens().size() > 0)
	{
		std::vector<std::string> tokens = this->_config.getTokens();
		if (tokens[0] == "autoindex")
			addAutoIndex();
		else if (tokens[0] == "can_upload")
			addCanUpload();
		else if (tokens[0] == "alias")
			addAlias();
		else if (tokens[0] == "return")
			addReturn();
		else if (tokens[0] == "upload_path")
			addUploadPath();
		else if (tokens[0] == "index")
			addIndex();
		else if (tokens[0] == "cgi_extensions")
			addCgiExtensions();
		else if (tokens[0] == "allow_methods")
			addAllowMethods();
		else if (tokens[0] == "}")
		{
			this->_config.removeTokens(1);
			break;
		}
		else {
			throw std::runtime_error("Invalid configuration: invalid token");
		}
	}
}

LocationBlock::~LocationBlock() {}

LocationBlock::LocationBlock(const LocationBlock &src)
	: _config(src._config),
	  _autoIndex(src._autoIndex),
	  _canUpload(src._canUpload),
	  _uri(src._uri),
	  _alias(src._alias),
	  _return(src._return),
	  _uploadPath(src._uploadPath),
	  _index(src._index),
	  _cgiExtensions(src._cgiExtensions),
	  _allowMethods(src._allowMethods)
{
}

LocationBlock &LocationBlock::operator=(const LocationBlock &src) {
	if (this != &src) {
		this->_autoIndex = src._autoIndex;
		this->_canUpload = src._canUpload;
		this->_uri = src._uri;
		this->_alias = src._alias;
		this->_return = src._return;
		this->_uploadPath = src._uploadPath;
		this->_index = src._index;
		this->_cgiExtensions = src._cgiExtensions;
		this->_allowMethods = src._allowMethods;
	}
	return *this;
}

bool LocationBlock::getAutoIndex() const { return this->_autoIndex; }
bool LocationBlock::getCanUpload() const { return this->_canUpload; }
std::string LocationBlock::getUri() const { return this->_uri; }
std::string LocationBlock::getAlias() const { return this->_alias; }
std::string LocationBlock::getReturn() const { return this->_return; }
std::string LocationBlock::getUploadPath() const { return this->_uploadPath; }
std::vector<std::string> LocationBlock::getIndex() const { return this->_index; }
std::vector<std::string> LocationBlock::getCgiExtensions() const { return this->_cgiExtensions; }
std::vector<std::string> LocationBlock::getAllowMethods() const { return this->_allowMethods; }

void LocationBlock::printLocationBlock()
{
	std::cout << "URI: " << this->_uri << std::endl;

	if (this->_autoIndex)
		std::cout << "Autoindex: true" << std::endl;
	else
		std::cout << "Autoindex: false" << std::endl;

	if (this->_canUpload)
		std::cout << "Can upload: true" << std::endl;
	else
		std::cout << "Can upload: false" << std::endl;

	std::cout << "Alias: " << this->_alias << std::endl;

	std::cout << "Return: " << this->_return << std::endl;

	std::cout << "Upload path: " << this->_uploadPath << std::endl;

	std::cout << "Index: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_index.begin(); it != this->_index.end(); ++it)
		std::cout << *it << std::endl;

	std::cout << "CGI extensions: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_cgiExtensions.begin(); it != this->_cgiExtensions.end(); ++it)
		std::cout << *it << std::endl;

	std::cout << "Allow methods: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_allowMethods.begin(); it != this->_allowMethods.end(); ++it)
		std::cout << *it << std::endl;
}

void LocationBlock::addAutoIndex()
{
	this->_config.removeTokens(1); //| Remove o token 'autoindex'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: autoindex: no autoindex value found");

	std::vector<std::string> tokens = this->_config.getTokens();
	if (tokens[0] == "on")
		this->_autoIndex = true;
	else if (tokens[0] == "off")
		this->_autoIndex = false;
	else
		throw std::runtime_error("Invalid configuration: autoindex: expected 'on' or 'off'");

	this->_config.removeTokens(1); //| Removendo o argumento de autoindex
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: autoindex: expected ';' at the end of autoindex directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCanUpload()
{
	this->_config.removeTokens(1); //| Remove o token 'can_upload'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: can_upload: no can_upload value found");

	std::vector<std::string> tokens = this->_config.getTokens();
	if (tokens[0] == "on")
		this->_canUpload = true;
	else if (tokens[0] == "off")
		this->_canUpload = false;
	else
		throw std::runtime_error("Invalid configuration: can_upload: expected 'on' or 'off'");

	this->_config.removeTokens(1); //| Removendo o argumento de can_upload
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: can_upload: expected ';' at the end of can_upload directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAlias()
{
	this->_config.removeTokens(1); //| Remove o token 'alias'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: alias: no alias value found");

	this->_config.verifyToken(END_OF_FILE, "Invalid configuration: alias: unexpected end of file");

	std::vector<std::string> tokens = this->_config.getTokens();
	this->_alias = tokens[0];

	this->_config.removeTokens(1); //| Removendo o argumento de alias
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: alias: expected ';' at the end of alias directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addReturn()
{
	this->_config.removeTokens(1); //| Remove o token 'return'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: return: no return value found");

	this->_config.verifyToken(END_OF_FILE, "Invalid configuration: return: unexpected end of file");

	std::vector<std::string> tokens = this->_config.getTokens();
	this->_return = tokens[0];

	this->_config.removeTokens(1); //| Removendo o argumento de return
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: return: expected ';' at the end of return directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addUploadPath()
{
	this->_config.removeTokens(1); //| Remove o token 'upload_path'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: upload_path: no upload_path value found");

	this->_config.verifyToken(END_OF_FILE, "Invalid configuration: upload_path: unexpected end of file");

	std::vector<std::string> tokens = this->_config.getTokens();
	this->_uploadPath = tokens[0];

	this->_config.removeTokens(1); //| Removendo o argumento de upload_path
	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: upload_path: expected ';' at the end of upload_path directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addIndex()
{
	this->_config.removeTokens(1); //| Remove o token 'index'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: index: no index value found");

	std::vector<std::string> indexes;
	while (this->_config.getTokens()[0] != ";")
	{
		this->_config.verifyToken(END_OF_FILE, "Invalid configuration: index: unexpected end of file");
		indexes.push_back(this->_config.getTokens()[0]);
		this->_config.removeTokens(1);
	}

	for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
		this->_index.push_back(*it);

	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: index: expected ';' at the end of index directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCgiExtensions()
{
	this->_config.removeTokens(1); //| Remove o token 'cgi_extensions'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: cgi_extensions: no cgi_extensions value found");

	std::vector<std::string> cgi_extensions;
	while (this->_config.getTokens()[0] != ";")
	{
		std::vector<std::string> tokens = this->_config.getTokens();
		this->_config.verifyToken(END_OF_FILE, "Invalid configuration: cgi_extensions: unexpected end of file");
		if (tokens[0][0] != '.') //| Adiciona o ponto na frente da extensão para ficar .php ou .py
			tokens[0] = "." + tokens[0];
		if (tokens[0] != ".php" && tokens[0] != ".py") //| Um dos bônus: multiplas extensões de cgi
			throw std::runtime_error("Invalid configuration: cgi_extensions: invalid extension");
		cgi_extensions.push_back(tokens[0]);
		this->_config.removeTokens(1);
	}

	for (std::vector<std::string>::iterator it = cgi_extensions.begin(); it != cgi_extensions.end(); ++it)
		this->_cgiExtensions.push_back(*it);

	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: cgi_extensions: expected ';' at the end of cgi_extensions directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAllowMethods()
{
	this->_config.removeTokens(1); //| Remove o token 'allow_methods'
	this->_config.verifyToken(SEMICOLON, "Invalid configuration: allow_methods: no allow_methods value found");

	std::vector<std::string> allow_methods;
	while (this->_config.getTokens()[0] != ";")
	{
		std::vector<std::string> tokens = this->_config.getTokens();
		this->_config.verifyToken(END_OF_FILE, "Configuração inválida: allow_methods: final do arquivo encontrado");
		if (tokens[0] != "GET" && tokens[0] != "POST" && tokens[0] != "DELETE")
			throw std::runtime_error("Configuração inválida: allow_methods: método inválido");
		allow_methods.push_back(tokens[0]);
		this->_config.removeTokens(1);
	}

	for (std::vector<std::string>::iterator it = allow_methods.begin(); it != allow_methods.end(); ++it)
		this->_allowMethods.push_back(*it);

	this->_config.verifyToken(DIFF_SEMICOLON, "Invalid configuration: allow_methods: expected ';' at the end of allow_methods directive");
	this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}
