/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:40:04 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/09 15:12:10 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include "../includes/Logger.hpp"
#include "../includes/StringUtils.hpp"

ServerConfig::ServerConfig(void) {}

ServerConfig::~ServerConfig(void) {}

ServerConfig::ServerConfig(int ac, char **av) {
	std::string configFile;

	if (ac == 2)
		configFile = av[1];
	else
		configFile = "config/default.conf";

	Logger::info("Attempting to read configuration file: " + configFile);

	this->parser(configFile);

	Logger::info("Configuration successfully read! Servers found: " +
                 StringUtils::size_tToString(this->_serverBlocks.size()));
}

void ServerConfig::readFile(const std::string &filename, std::string &content)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + filename);

	content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void ServerConfig::trim(std::string &content)
{
	size_t start = content.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos)
	{
		content.clear();
		return;
	}

	size_t end = content.find_last_not_of(" \t\n\r\f\v");

	content = content.substr(start, end - start + 1);
}

void ServerConfig::removeComments(std::string &content)
{
	std::string result;
	std::istringstream iss(content);
	std::string line;

	while (std::getline(iss, line))
	{
		size_t commentPos = line.find('#');

		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos);

		result += line + "\n";
	}

	if (!result.empty() && result[result.length() - 1] == '\n')
		result.erase(result.length() - 1);

	content = result;
}

void ServerConfig::cleanFile(const std::string &filename, std::string &content)
{
	readFile(filename, content); //| Arquivo completo
	removeComments(content);     //| Remover comentários (linhas com '#')
	trim(content);               //| Remover os whitespaces do começo e do final do content.
}

std::vector<std::string> ServerConfig::tokenizeContent(const std::string &content)
{
	std::string currentToken;
	bool inQuotes = false;
	char quoteChar = '\0';
	int braceCount = 0;

	for (size_t i = 0; i < content.length(); ++i)
	{
		char c = content[i];

		if ((c == '"' || c == '\'') && !inQuotes) //| Verifica se o caractere é uma aspas
		{
			inQuotes = true;
			quoteChar = c;
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
			currentToken += c;
		}
		else if (c == quoteChar && inQuotes) //| Verifica se o caractere é a aspas que fecha
		{
			inQuotes = false;
			currentToken += c;
			this->_tokens.push_back(currentToken);
			currentToken.clear();
			quoteChar = '\0';
		}
		else if (inQuotes) //| Verifica se o caractere está dentro de aspas, se estiver, só passa para o próximo caractere
			currentToken += c;
		else if (c == '{' || c == '}' || c == ';') //| Verifica se o caractere é uma chave ou ponto e vírgula
		{
			if (!inQuotes) //| Conta a quantidade de chaves abertas e fechadas
			{
				if (c == '{')
					braceCount++;
				else if (c == '}')
					braceCount--;
			}
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
			this->_tokens.push_back(std::string(1, c));
		}
		else if (std::isspace(c)) //| Verifica se o caractere é um espaço
		{
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
		}
		else //| Se não for um espaço, adiciona o caractere ao token atual
			currentToken += c;
	}

	if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
		this->_tokens.push_back(currentToken);

	if (braceCount != 0) //| Verifica se as chaves estão balanceadas
		throw std::runtime_error("Invalid configuration: unbalanced braces");

	return this->_tokens;
}

void ServerConfig::parser(const std::string &filename)
{
	std::string content;
	cleanFile(filename, content);

	Logger::info("DEBUG: Cleaned file. Content size: " +
			StringUtils::size_tToString(content.size()));

	tokenizeContent(content);

	Logger::info("DEBUG: Tokens generated: " +
				StringUtils::size_tToString(this->_tokens.size()));
	if (this->_tokens.size() > 0)
		Logger::info("DEBUG: First token: " + this->_tokens[0]);

	if (this->_tokens.size() == 0)
		throw std::runtime_error("Invalid configuration: no server found");

	while (this->_tokens.size() > 0)
	{
		Logger::info("DEBUG: Processing token: " + this->_tokens[0]);

		if (this->_tokens[0] == "server" && this->_tokens[1] == "{")
			this->_serverBlocks.push_back(ServerBlock(*this));
		else
			throw std::runtime_error("Invalid configuration: server not found");
	}
}

void ServerConfig::removeTokens(size_t amount)
{
	if (!this->_tokens.empty())
		this->_tokens.erase(this->_tokens.begin(), this->_tokens.begin() + amount);
}

void ServerConfig::verifyToken(TypeValidation type, const std::string &message)
{
	bool shouldThrow = false;

	switch (type)
	{
		case EMPTY:
			shouldThrow = this->_tokens.empty();
			break;
		case SEMICOLON:
			shouldThrow = this->_tokens.empty() || this->_tokens[0] == ";";
			break;
		case DIFF_SEMICOLON:
			shouldThrow = this->_tokens.empty() || this->_tokens[0] != ";";
			break;
		case END_OF_FILE:
			shouldThrow = this->_tokens[0] == this->_tokens.back();
			break;
		default:
			throw std::runtime_error("Invalid configuration: unknown validation type");
	}

	if (shouldThrow)
		throw std::runtime_error(message);
}

std::vector<std::string> ServerConfig::getTokens(void)
{
	return this->_tokens;
}

std::vector<ServerBlock> ServerConfig::getServerBlocks(void) const
{
	return this->_serverBlocks;
}
