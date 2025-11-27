/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:24 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 09:40:57 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/ServerManage.hpp"
#include "../includes/EpollInstance.hpp"
#include "../includes/Runtime.hpp"

Client::~Client() {}

Client::Client(int clientFd, ServerManage &server)
	: EpollHandler(EPOLLIN | EPOLLRDHUP, clientFd, 30), _server(server)
{
	this->_state = STATE_READING_HEADER;
	this->_rawRequest = "";
	this->_request = HttpRequest();
	this->_response = HttpResponse();
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
		this->_request = src._request;
		this->_response = src._response;
		this->_state = src._state;
		this->_rawRequest = src._rawRequest;
		this->_pendingResponse = src._pendingResponse;
		this->_responseOffset = src._responseOffset;
		this->setSocketFd(src.getSocketFd());
	}
	return (*this);
}

// GETTERS
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
	return (this->_request);
}

HttpResponse &Client::getResponse(void)
{
	return (this->_response);
}

// SETTERS
void Client::setState(int state)
{
	this->_state = state;
}

// Concatena dados recebidos à requisição
void Client::concatenateRequestData(const std::string &data)
{
	this->_rawRequest += data;
	
	// Se ainda está lendo headers
	if (this->_state == STATE_READING_HEADER)
	{
		size_t headerEnd = this->_rawRequest.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			// Headers completos, muda para leitura de body
			this->setState(STATE_READING_BODY);
			
			// Verifica se há Content-Length
			size_t contentLengthPos = this->_rawRequest.find("Content-Length:");
			if (contentLengthPos != std::string::npos)
			{
				// Extrai valor do Content-Length
				size_t valueStart = contentLengthPos + 15;
				size_t lineEnd = this->_rawRequest.find("\r\n", valueStart);
				std::string lengthStr = this->_rawRequest.substr(valueStart, lineEnd - valueStart);
				
				// Remove espaços
				while (!lengthStr.empty() && isspace(lengthStr[0]))
					lengthStr.erase(0, 1);
				
				int contentLength = std::atoi(lengthStr.c_str());
				size_t bodyStart = headerEnd + 4;
				size_t currentBodySize = this->_rawRequest.size() - bodyStart;
				
				// Verifica se recebeu todo o body
				if (currentBodySize >= static_cast<size_t>(contentLength))
					this->setState(STATE_COMPLETE);
			}
			else
			{
				// Sem Content-Length, requisição completa
				this->setState(STATE_COMPLETE);
			}
		}
	}
}

// Verifica se a requisição HTTP está completa
bool Client::isRequestComplete(void)
{
	return (this->_state == STATE_COMPLETE);
}

// Processa a requisição HTTP
void Client::processRequest(void)
{
	try
	{
		HttpParse req = this->_request.httpParse(this->_rawRequest);
		
		if (req.uri.empty() || req.uri[0] != '/')
		{
			this->_response.setErrorPage(404);
			return;
		}

		if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		{
			this->_response.setErrorPage(405);
			return;
		}

		this->_response = this->_response.dispatchRequest(req);
	}
	catch (std::exception &error)
	{
		std::cout << "Erro ao processar requisição: " << error.what() << std::endl;
		this->_response.setErrorPage(400);
	}
}

// Envia a resposta HTTP (com suporte a envio parcial)
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
			// Socket não está pronto, adicionar EPOLLOUT
			struct epoll_event ev;
			ev.events = EPOLLOUT | EPOLLRDHUP;
			ev.data.ptr = this;
			epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_MOD, this->getSocketFd(), &ev);
			return false;
		}
		// Erro real
		std::cerr << "Erro ao enviar resposta: " << strerror(errno) << std::endl;
		return false;
	}

	this->_responseOffset += bytesSent;

	// Verifica se enviou tudo
	if (this->_responseOffset >= this->_pendingResponse.size())
	{
		this->_pendingResponse.clear();
		this->_responseOffset = 0;
		return true;
	}

	// Ainda há dados pendentes, precisa de EPOLLOUT
	struct epoll_event ev;
	ev.events = EPOLLOUT | EPOLLRDHUP;
	ev.data.ptr = this;
	epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_MOD, this->getSocketFd(), &ev);
	return false;
}

// Handler para eventos EPOLLIN (dados disponíveis para leitura)
void Client::EpollInHandler(void)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesRead = recv(this->getSocketFd(), buffer, BUFFER_SIZE - 1, 0);

	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			std::cout << "Cliente desconectou (fd=" << this->getSocketFd() << ")" << std::endl;
		else
			std::cout << "Erro na leitura do cliente (fd=" << this->getSocketFd() << ")" << std::endl;
		
		this->deleteHandler();
		return;
	}

	buffer[bytesRead] = '\0';
	this->concatenateRequestData(std::string(buffer));

	// Verifica se a requisição está completa
	if (this->isRequestComplete())
	{
		std::cout << "Requisição completa recebida (fd=" << this->getSocketFd() << ")" << std::endl;
		
		// Processa a requisição
		this->processRequest();
		
		// Tenta enviar resposta
		std::string responseStr = this->_response.toString();
		
		if (this->sendResponse(responseStr))
		{
			// Resposta enviada completamente
			std::cout << "Resposta enviada com sucesso (fd=" << this->getSocketFd() 
					  << ", status=" << this->_response.status_code << ")" << std::endl;
			this->deleteHandler();
		}
		// Se não enviou tudo, EPOLLOUT foi configurado em sendResponse()
	}
}

// Handler para eventos EPOLLOUT (socket pronto para escrita)
void Client::EpollOutHandler(void)
{
	if (this->_pendingResponse.empty())
		return;

	if (this->sendResponse(this->_pendingResponse))
	{
		// Resposta enviada completamente
		std::cout << "Resposta enviada com sucesso (fd=" << this->getSocketFd() 
				  << ", status=" << this->_response.status_code << ")" << std::endl;
		this->deleteHandler();
	}
	// Se ainda há dados pendentes, EPOLLOUT permanece ativo
}

// Limpa e remove o handler do epoll
void Client::deleteHandler(void)
{
	std::cout << "Removendo cliente (fd=" << this->getSocketFd() << ")" << std::endl;
	
	epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, this->getSocketFd(), NULL);
	close(this->getSocketFd());
	
	// Remove do mapa de clientes no Runtime
	RunTime::getClients().erase(this->getSocketFd());
}
