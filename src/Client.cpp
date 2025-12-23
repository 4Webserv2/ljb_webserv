/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:24 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/22 21:39:04 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/ServerManage.hpp"
#include "../includes/EpollInstance.hpp"
#include "../includes/Runtime.hpp"
#include "../includes/Logger.hpp"
#include "../includes/StringUtils.hpp"

Client::~Client() {}

Client::Client(int clientFd, ServerManage &server)
	: EpollHandler(clientFd, EPOLLIN | EPOLLRDHUP, 30), _server(server)
{
	this->_state = STATE_READING_HEADER;
	this->_rawRequest = "";
	this->request = HttpRequest();
	this->response = HttpResponse();
	this->_pendingResponse = "";
	this->_responseOffset = 0;
}

Client::Client(const Client &src)
	: EpollHandler(src.getSocketFd(), src.getActiveEvents(), src.getEventsTimeout()),
	  _server(src._server)
{
	*this = src;
}

Client &Client::operator=(const Client &src)
{
	if (this != &src)
	{
		this->request = src.request;
		this->response = src.response;
		this->_state = src._state;
		this->_rawRequest = src._rawRequest;
		this->_pendingResponse = src._pendingResponse;
		this->_responseOffset = src._responseOffset;
		this->setSocketFd(src.getSocketFd());
	}
	return (*this);
}

int Client::getState(void) const
{
	return (this->_state);
}

std::string &Client::getRawRequest(void)
{
	return (this->_rawRequest);
}

HttpRequest &Client::getRequest(void)
{
	return (this->request);
}

HttpResponse &Client::getResponse(void)
{
	return (this->response);
}

void Client::setState(int state)
{
	this->_state = state;
}

void Client::concatenateRequestData(const std::string &data)
{
	this->_rawRequest += data;

	std::cout << "[Client] Dados recebidos: +" << data.size() << " bytes (total: " << this->_rawRequest.size() << ")" << std::endl;

	if (this->_state == STATE_READING_HEADER)
	{
		size_t headerEnd = this->_rawRequest.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			this->setState(STATE_READING_BODY);
			std::cout << "[Client] ✅ Headers completos! Mudando para STATE_READING_BODY" << std::endl;
		}
	}

	if (this->_state == STATE_READING_BODY)
	{
		// Encontrar fim dos headers
		size_t headerEnd = this->_rawRequest.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			std::cerr << "[Client] ❌ ERRO: STATE_READING_BODY mas headers não encontrados!" << std::endl;
			return;
		}

		// Extrair apenas a parte de headers para parsing rápido
		std::string headersOnly = this->_rawRequest.substr(0, headerEnd + 4);

		// Procurar manualmente por Content-Length nos headers
		std::string contentLengthStr;
		std::string transferEncoding;

		std::istringstream headerStream(headersOnly);
		std::string line;

		// Pular primeira linha (request line)
		std::getline(headerStream, line);

		// Ler headers
		while (std::getline(headerStream, line))
		{
			// Remover \r se existir
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			if (line.empty())
				break;

			// Procurar por Content-Length ou Transfer-Encoding
			if (line.find("content-length:") == 0 || line.find("Content-Length:") == 0)
			{
				size_t colonPos = line.find(':');
				contentLengthStr = line.substr(colonPos + 1);
				// Trim
				while (!contentLengthStr.empty() && std::isspace(contentLengthStr[0]))
					contentLengthStr.erase(0, 1);
				while (!contentLengthStr.empty() && std::isspace(contentLengthStr[contentLengthStr.length() - 1]))
					contentLengthStr.erase(contentLengthStr.length() - 1);
			}

			if (line.find("transfer-encoding:") == 0 || line.find("Transfer-Encoding:") == 0)
			{
				size_t colonPos = line.find(':');
				transferEncoding = line.substr(colonPos + 1);
				// Trim
				while (!transferEncoding.empty() && std::isspace(transferEncoding[0]))
					transferEncoding.erase(0, 1);
				while (!transferEncoding.empty() && std::isspace(transferEncoding[transferEncoding.length() - 1]))
					transferEncoding.erase(transferEncoding.length() - 1);
			}
		}

		// Verificar se é chunked
		bool isChunked = (transferEncoding.find("chunked") != std::string::npos);

		if (isChunked)
		{
			std::cout << "[Client] Transfer-Encoding: chunked" << std::endl;
			// Para chunked, verificar se termina com "0\r\n\r\n"
			if (this->_rawRequest.find("\r\n0\r\n\r\n") != std::string::npos ||
				this->_rawRequest.find("\n0\n\n") != std::string::npos)
			{
				std::cout << "[Client] ✅ Chunked encoding completo!" << std::endl;
				this->setState(STATE_COMPLETE);
			}
			else
			{
				std::cout << "[Client] ⏳ Chunked: aguardando último chunk (0)..." << std::endl;
			}
		}
		else if (!contentLengthStr.empty())
		{
			// Usar Content-Length
			int contentLength = std::atoi(contentLengthStr.c_str());
			size_t bodyStart = headerEnd + 4;
			size_t currentBodySize = this->_rawRequest.size() - bodyStart;

			std::cout << "[Client] 📊 Content-Length: " << contentLength << " bytes" << std::endl;
			std::cout << "[Client] 📊 Body atual: " << currentBodySize << " bytes" << std::endl;
			std::cout << "[Client] 📊 Progresso: " << (currentBodySize * 100 / (contentLength > 0 ? contentLength : 1)) << "%" << std::endl;

			if (currentBodySize >= (size_t)contentLength)
			{
				std::cout << "[Client] ✅ Body completo recebido!" << std::endl;
				this->setState(STATE_COMPLETE);
			}
			else
			{
				std::cout << "[Client] ⏳ Faltam " << (contentLength - currentBodySize) << " bytes..." << std::endl;
			}
		}
		else
		{
			// Sem Content-Length e sem chunked - assumir completo
			std::cout << "[Client] ℹ️ Sem Content-Length/chunked, assumindo completo" << std::endl;
			this->setState(STATE_COMPLETE);
		}
	}
}

bool Client::isRequestComplete(void)
{
	return (this->_state == STATE_COMPLETE);
}

void Client::processRequest(void)
{
	try
	{
		HttpRequest req;
		req.setPar(this->request.httpParse(this->_rawRequest));

		// Configurar error pages do ServerBlock (usar referências para evitar cópias)
		const ServerBlock &block = this->_server.getBlock();
		const std::map<int, std::string> &errorPages = block.getErrorPages();
		std::string rootPath = block.getRoot().second;

		this->response.setErrorPageConfig(&errorPages, rootPath);

		// Validar URI
		if (req.getUri().empty() || req.getUri()[0] != '/')
		{
			this->response.setErrorPage(404);
			return;
		}
		if (req.getMethod() != "GET" &&
			req.getMethod() != "POST" &&
			req.getMethod() != "DELETE")
		{
			std::string reqMethod = StringUtils::ostreamToString(req.getMethod());
			Logger::error("Method not allowed: " + reqMethod);
			this->response.setErrorPage(405);
			return;
		}

		// Procurar melhor match sem copiar LocationBlock (usar ponteiro para o entry const)
		std::string uri = req.getUri();
		std::map<std::string, LocationBlock> locations = block.getLocations();

		if (locations.empty())
		{
			Logger::error("No location blocks configured");
			this->response.setErrorPage(500);
			return;
		}

		std::string bestMatch = "";
		const LocationBlock *bestLocationPtr = NULL;

		for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
			 it != locations.end(); ++it)
		{
			const std::string &path = it->first;
			size_t pathLen = path.size();
			// evitar comparar além do tamanho da URI e entradas vazias
			if (pathLen == 0 || uri.size() < pathLen)
				continue;
			if (uri.compare(0, pathLen, path) == 0)
			{
				if (pathLen > bestMatch.size())
				{
					bestMatch = path;
					bestLocationPtr = &(it->second);
				}
			}
		}

		if (bestLocationPtr == NULL)
		{
			Logger::warning("No location block found for URI: " + uri);
			this->response.setErrorPage(404);
			return;
		}

		Logger::debug("Best location match: " + bestMatch);

		// Processar requisição passando referência (sem cópia)
		this->response = this->response.dispatchRequest(req, *bestLocationPtr);

		// Manter a configuração de error pages após dispatch
		this->response.setErrorPageConfig(&errorPages, rootPath);
	}
	catch (std::exception &error)
	{
		Logger::error(std::string("Error processing request: ") + error.what());
		ServerBlock block = this->_server.getBlock();
		std::map<int, std::string> errorPages = block.getErrorPages();
		std::string rootPath = block.getRoot().second;
		this->response.setErrorPageConfig(&errorPages, rootPath);

		this->response.setErrorPage(400);
	}
}

bool Client::sendResponse(const std::string &responseStr)
{
	if (this->_pendingResponse.empty())
	{
		this->_pendingResponse = responseStr;
		this->_responseOffset = 0;
	}

	ssize_t bytesSent = send(
		this->getSocketFd(),
		this->_pendingResponse.c_str() + this->_responseOffset,
		this->_pendingResponse.size() - this->_responseOffset,
		MSG_NOSIGNAL);

	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			uint32_t events = this->getActiveEvents();
			events |= EPOLLOUT;
			this->setActiveEvents(events);
			EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
			return false;
		}
		Logger::error("Error sending response: " + std::string(strerror(errno)));
		return false;
	}

	this->_responseOffset += bytesSent;
	if (this->_responseOffset >= this->_pendingResponse.size())
	{
		this->_pendingResponse.clear();
		this->_responseOffset = 0;
		return true;
	}
	uint32_t events = this->getActiveEvents();
	events |= EPOLLOUT;
	this->setActiveEvents(events);
	EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
	return false;
}

#include <iostream>

void Client::EpollInHandler(void)
{
	char buffer[4096] = {0};
	int count = 0;

	if ((count = read(this->getSocketFd(), buffer, sizeof(buffer))) > 0)
	{
		this->concatenateRequestData(std::string(buffer, count));
		if (this->isRequestComplete())
		{
			try
			{
				this->request.setPar(this->request.httpParse(this->_rawRequest));
			}
			catch (std::exception &error)
			{
				Logger::error("Failed to parse HTTP request");
				this->response.setErrorPage(400);
				std::string responseStr = this->response.toString();
				if (!sendResponse(responseStr))
					return;
				if (this->_pendingResponse.empty())
				{
					EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
					return;
				}
				return;
			}

			Logger::debug("===== REQUEST COMPLETE =====");
			Logger::debug(this->request.getMethod() + " " + this->request.getUri());

			std::map<std::string, std::string> headers = this->request.getHeaders();
			for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
				Logger::debug(it->first + ": " + it->second);

			Logger::debug("Body: " + this->request.getBody());
			Logger::debug("============================");
			Logger::info("deu merda aqui @@@@@@");
			// CORRIGIDO: Usar cópia ao invés de ponteiro
			const ServerBlock &block = this->_server.getBlock();
			Logger::info("aqui deu merda tbm");
			const std::map<int, std::string> &errorPages = block.getErrorPages();
			Logger::info("aqui deu merda tbm2");
			std::string rootPath = block.getRoot().second;
			Logger::info("aqui deu merda tbm3");
			this->response.setErrorPageConfig(&errorPages, rootPath);
			Logger::info("aqui deu merda tbm4");

			std::string uri = this->request.getUri();
			Logger::info("aqui deu merda tbm5");
			std::map<std::string, LocationBlock> locations = block.getLocations();
			Logger::info("aqui deu merda tbm6");
			if (locations.empty())
			{
				Logger::error("No location blocks configured");
				Logger::info("teste de erro");
				this->response.setErrorPage(500);
			}
			else
			{
				// CORRIGIDO: Procurar melhor match SEM inicializar bestLocation antecipadamente
				std::string bestMatch = "";
				const LocationBlock *bestLocationPtr = NULL;

				//ERRO NESSE FOR
				for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin(); it != locations.end(); ++it)
				{
					const std::string &path = it->first;
					size_t pathLen = path.size();
					// evitar comparar além do tamanho da URI e entradas vazias
					if (pathLen == 0 || uri.size() < pathLen)
						continue;
					if (uri.compare(0, pathLen, path) == 0)
					{
						if (pathLen > bestMatch.size())
						{
							bestMatch = path;
							bestLocationPtr = &(it->second);
						}
					}
				}
				std::cout << "CARALHOOOOOOOOOOOOOOO" << std::endl;

				if (bestLocationPtr != NULL)
				{
					Logger::debug("Best location match: " + bestMatch);

					// Passar referência direta para evitar copia/desalocação
					this->response = this->response.dispatchRequest(this->request, *bestLocationPtr);
				}
				else
				{
					Logger::warning("No location block found for URI: " + uri);
					this->response.setErrorPage(404);
				}
			}
			Logger::info("aqui deu merda tbm8");
			std::string responseStr = this->response.toString();
			Logger::info("aqui deu merda tbm9");
			Logger::debug("===== RESPONSE SEND =====");
			//Logger::debug(responseStr);
			Logger::debug("=========================");

			if (!sendResponse(responseStr))
				return;
			if (this->_pendingResponse.empty())
			{
				EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
				return;
			}
		}
	}
	else if (count == 0)
	{
		Logger::info("Client closed the connection.");
		std::cout << this->getRawRequest() << std::endl;
		RunTime::deleteClient(this->getSocketFd());
	}
}

void Client::EpollOutHandler(void)
{
	if (this->_pendingResponse.empty() || this->_responseOffset >= this->_pendingResponse.size())
	{
		uint32_t events = this->getActiveEvents();
		events &= ~EPOLLOUT;
		this->setActiveEvents(events);
		EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
		return;
	}
	if (!sendResponse(this->_pendingResponse))
		return;
	if (this->_responseOffset >= this->_pendingResponse.size())
	{
		uint32_t events = this->getActiveEvents();
		events &= ~EPOLLOUT;
		this->setActiveEvents(events);
		EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
		EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
		return;
	}
}

void Client::deleteHandler(void)
{
	Logger::info("Removing client (fd=" + StringUtils::intToString(this->getSocketFd()) + ")");

	epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, this->getSocketFd(), NULL);
	close(this->getSocketFd());
	RunTime::getClients().erase(this->getSocketFd());
}
