/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServeListen.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 20:58:51 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:17:25 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerListen.hpp"

ServerListen::~ServerListen(){}

ServerListen::ServerListen(unsigned int host, int port, const ServerBlock &block)
	: EpollHandler(EPOLLIN | EPOLLRDHUP),
	_host(host), _port(port), _block(block) {}

ServerListen::ServerListen(const ServerListen &src)
	: EpollHandler(src.getSocketFd(), src.getActiveEvents()),
	_host(src._host), _port(src._port), _block(src._block) {}

ServerListen &ServerListen::operator=(const ServerListen &src)
{
	if (this != &src)
	{
		this->_host = src._host;
		this->_port = src._port;
		this->setSocketFd(src.getSocketFd());
	}
	return (*this);
}

void ServerListen::startSocket(int domain, int type)
{
	this->makeSocket(domain, type);
	this->setServerAddr(domain);
	this->bindServer();
}

void	ServerListen::makeSocket(int domain, int type)
{
	int	initSocket = socket(domain, type, 0);
	if (initSocket == -1)
	{
		std::cout << "Cannot init Server socket!" << std::endl;
		return ;
	}
	this->setSocketFd(initSocket);
}

void	ServerListen::bindServer(void)
{
	int initBind = bind(this->getSocketFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr));
	if (initBind == -1)
	{
		std::cout << "Cannot bind Server socket!" << std::endl;
		return ;
	}
}

void ServerListen::setServerAddr(int domain)
{
	this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
	this->_serverAddr.sin_family = domain;
	this->_serverAddr.sin_port = htons(this->getPort());
}

void ServerListen::listenSocket(void)
{
	int initListen = listen(this->getSocketFd(), MAX_EVENTS);
	if (initListen == -1)
	{
		std::cout << "Cannot listen to Server socket!" << std::endl;
		return ;
	}
}

unsigned int ServerListen::getHost() const
{
	return (this->_host);
}

int ServerListen::getPort() const
{
	return (this->_port);
}

ServerBlock ServerListen::getBlock() const
{
	return (this->_block);
}
