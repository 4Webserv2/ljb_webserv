/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 20:58:51 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 14:56:31 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerManage.hpp"

ServerManage::~ServerManage(){}

ServerManage::ServerManage(unsigned int host, int port, const ServerBlock &block)
	: EpollHandler(EPOLLIN | EPOLLRDHUP),
	_host(host), _port(port), _block(block) {}

ServerManage::ServerManage(const ServerManage &src)
	: EpollHandler(src.getSocketFd(), src.getActiveEvents()),
	_host(src._host), _port(src._port), _block(src._block) {}

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

void ServerManage::startSocket(int domain, int type)
{
	this->makeSocket(domain, type);
	this->setServerAddr(domain);
	this->bindServer();
}

void	ServerManage::makeSocket(int domain, int type)
{
	int	initSocket = socket(domain, type, 0);
	if (initSocket == -1)
	{
		std::cout << "Cannot init Server socket!" << std::endl;
		return ;
	}
	this->setSocketFd(initSocket);
}

void	ServerManage::bindServer(void)
{
	int initBind = bind(this->getSocketFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr));
	if (initBind == -1)
	{
		std::cout << "Cannot bind Server socket!" << std::endl;
		return ;
	}
}

void ServerManage::setServerAddr(int domain)
{
	this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
	this->_serverAddr.sin_family = domain;
	this->_serverAddr.sin_port = htons(this->getPort());
}

void ServerManage::listenSocket(void)
{
	int initListen = listen(this->getSocketFd(), MAX_EVENTS);
	if (initListen == -1)
	{
		std::cout << "Cannot listen to Server socket!" << std::endl;
		return ;
	}
}

unsigned int ServerManage::getHost() const
{
	return (this->_host);
}

int ServerManage::getPort() const
{
	return (this->_port);
}

ServerBlock ServerManage::getBlock() const
{
	return (this->_block);
}

int ServerManage::getFd() const
{
	return (this->getSocketFd());
}
