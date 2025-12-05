/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:24 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/05 16:58:48 by lraggio          ###   ########.fr       */
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
	: EpollHandler(src.getActiveEvents(), src.getSocketFd(), src.getEventsTimeout()),
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
	if (this->_state == STATE_READING_HEADER)
	{
		size_t headerEnd = this->_rawRequest.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			this->setState(STATE_READING_BODY);

			// Parse parcial para pegar Content-Length
			HttpRequest tempReq;
			HttpParse tempParse = tempReq.httpParse(this->_rawRequest);

			// Usar getter case-insensitive
			std::string contentLengthStr = tempReq.getHeader("Content-Length");

			if (!contentLengthStr.empty())
			{
				int contentLength = std::atoi(contentLengthStr.c_str());
				size_t bodyStart = headerEnd + 4;
				size_t currentBodySize = this->_rawRequest.size() - bodyStart;

				if (currentBodySize >= (size_t)contentLength)
					this->setState(STATE_COMPLETE);
			}
			else
			{
				// Sem Content-Length, request está completo
				this->setState(STATE_COMPLETE);
			}
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

		// Configurar error pages do ServerBlock
		ServerBlock block = this->_server.getBlock();
		std::map<int, std::string> errorPages = block.getErrorPages();
		std::string rootPath = block.getRoot().second;

		this->response.setErrorPageConfig(&errorPages, rootPath);

		// Validar URI
		if (req.getUri().empty() || req.getUri()[0] != '/')
		{
			this->response.setErrorPage(404);
			return;
		}

		// Validar método (retorna 405 se não suportado)
		if (req.getMethod() != "GET" &&
			req.getMethod() != "POST" &&
			req.getMethod() != "DELETE")
		{
			std::string reqMethod = StringUtils::intToString(req.getMethod());
			Logger::error("Method not allowed: " << reqMethod);
			this->response.setErrorPage(405);
			return;
		}

		// Processar requisição
		this->response = this->response.dispatchRequest(req);

		// Manter a configuração de error pages após dispatch
		this->response.setErrorPageConfig(&errorPages, rootPath);
	}
	catch (std::exception &error)
	{
		std::cerr << "Error processing request: " << error.what() << std::endl;

		// Garantir que error page config está disponível
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
		MSG_NOSIGNAL
	);

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
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
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

void Client::EpollInHandler(void)
{
	char buffer[4096] = {0};
	int count = 0;

	if ((count = read(this->getSocketFd(), buffer, sizeof(buffer))) > 0)
	{
		this->concatenateRequestData(std::string(buffer, count));
		if (this->isRequestComplete())
		{
			try {
				this->request.setPar(this->request.httpParse(this->_rawRequest));
			}
			catch (std::exception &error) {
				this->response.setErrorPage(400);
				std::string responseStr = this->response.toString();
				if (!sendResponse(responseStr))
					return;
				if (this->_pendingResponse.empty())
					RunTime::deleteClient(this->getSocketFd());
				return;
			}
			std::cout << "----------------- REQUEST COMPLETE ------------------" << std::endl;
			std::cout << this->request.getMethod() << std::endl;
			std::cout << this->request.getUri() << std::endl;
			std::map<std::string, std::string> headers = this->request.getHeaders();
			for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
				std::cout << it->first << ": " << it->second << std::endl;
			std::cout << "Body: " << this->request.getBody() << std::endl;
			std::cout << "-----------------------------------------------------" << std::endl;
			this->response = this->response.dispatchRequest(this->request);
			std::string responseStr = this->response.toString();
			std::cout << "------------------- RESPONSE SEND -------------------" << std::endl;
			Logger::info(responseStr);
			std::cout << "-----------------------------------------------------" << std::endl;
			if (!sendResponse(responseStr))
				return;
			if (this->_pendingResponse.empty())
				RunTime::deleteClient(this->getSocketFd());
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
		RunTime::deleteClient(this->getSocketFd());
	}
}

void Client::deleteHandler(void)
{
	Logger::info("Removing client (fd=" + StringUtils::intToString(this->getSocketFd()) + ")");

	epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, this->getSocketFd(), NULL);
	close(this->getSocketFd());
	RunTime::getClients().erase(this->getSocketFd());
}
