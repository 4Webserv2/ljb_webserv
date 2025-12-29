/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:51:28 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/28 23:00:47 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"

ServerManage::ServerManage(unsigned int host, int port, const ServerBlock &serverBlock)
	: EpollHandler(EPOLLIN | EPOLLRDHUP), _host(host), _port(port), _serverBlock(serverBlock)
{
	initServerSocket(AF_INET, SOCK_STREAM);
}

ServerManage::ServerManage(const ServerManage &src)
	: EpollHandler(src.getInterestedEvents(), src.getSocketFd()), _host(src._host), _port(src._port), _serverBlock(src._serverBlock) {}

ServerManage &ServerManage::operator=(const ServerManage &src)
{
	if (this != &src)
	{
		this->_host = src._host;
		this->_port = src._port;
		this->setSocketFd(src.getSocketFd());
	}
	return (*this);
}

bool ServerManage::operator==(const ServerManage &other) const
{
	return (this->_host == other._host && this->_port == other._port && this->getSocketFd() == other.getSocketFd());
}

ServerManage::~ServerManage(void) {}

void ServerManage::handleEpollIn(void)
{
	while (true)
	{
		struct sockaddr_in clientSocketAddr;
		socklen_t clientSocketLength = sizeof(clientSocketAddr);
		int clientFd = accept(this->getSocketFd(), (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

		if (clientFd == -1)
			break;
		else
		{
			try
			{
				setNonBlocking(clientFd);
				int flag = 1;
				if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0)
					Logger::error("Failed to set TCP_NODELAY on client socket");
				EpollInstance::manipInterestList(EPOLL_CTL_ADD, new Client(clientFd, *this));
			}
			catch (const std::exception &e)
			{
				Logger::error("Exception caught in ServerManage::handleEpollIn: " + std::string(e.what()));
				close(clientFd);
			}
		}
	}
}

unsigned int ServerManage::getHost(void) const
{
	return (this->_host);
}

int ServerManage::getPort(void) const
{
	return (this->_port);
}

ServerBlock ServerManage::getServerBlock(void) const
{
	return (this->_serverBlock);
}

void ServerManage::setServerAddr(int socketDomain)
{
	this->_serverAddr.sin_family = socketDomain;
	this->_serverAddr.sin_port = htons(this->getPort());
	this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
}

void ServerManage::createServerSocket(int socketDomain, int socketType)
{
	this->setSocketFd(socket(socketDomain, socketType, 0));

	if (this->getSocketFd() == -1)
		throw(ServerManage::CannotInitServerSocket());
}

void ServerManage::bindServerSocket(void)
{
	if (bind(this->getSocketFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1)
		throw(ServerManage::CannotBindServerSocket());
}

void ServerManage::updateToNonBlocking(void)
{
	try
	{
		setNonBlocking(this->getSocketFd());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}

void ServerManage::listenServerSocket(void)
{
	if (listen(this->getSocketFd(), MAX_EVENTS) == -1)
		throw(ServerManage::CannotSetServerToListen());

}

void ServerManage::allowAddrReuse(void)
{
	int option = 1;

	if (setsockopt(this->getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
		throw(ServerManage::CannotAllowAddrReuse());

}

void ServerManage::initServerSocket(int socketDomain, int socketType)
{
	this->createServerSocket(socketDomain, socketType);
	this->setServerAddr(socketDomain);
	this->allowAddrReuse();
	this->bindServerSocket();
	this->updateToNonBlocking();
	this->listenServerSocket();
}

const char *ServerManage::CannotInitServerSocket::what() const throw()
{
	return ("Error: error in creating socket with socket().");
}

const char *ServerManage::CannotBindServerSocket::what() const throw()
{
	return ("Error: error in binding server socket with bind().");
}

const char *ServerManage::CannotUpdateServerToNonBlocking::what() const throw()
{
	return ("Error: error in trying to set the non-blocking behavior.");
}

const char *ServerManage::CannotSetServerToListen::what() const throw()
{
	return ("Error: error in setting the server to listen with listen().");
}

const char *ServerManage::CannotAllowAddrReuse::what() const throw()
{
	return ("Error: error in allowing address reuse with setsockopt().");
}
