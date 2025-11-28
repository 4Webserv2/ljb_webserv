/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 20:58:51 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 22:11:59 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerManage.hpp"
#include "../includes/Client.hpp"
#include "../includes/Runtime.hpp"
#include "../includes/EpollInstance.hpp"

ServerManage::~ServerManage(){}

ServerManage::ServerManage(unsigned int host, int port, const ServerBlock &block)
    : EpollHandler(EPOLLIN | EPOLLRDHUP), _block(block)
{
    this->_host = host;
    this->_port = port;
    memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
}

ServerManage::ServerManage(const ServerManage &src)
    : EpollHandler(src.getSocketFd(), src.getActiveEvents(), src.getEventsTimeout()),
      _block(src._block)
{
    *this = src;
}

ServerManage &ServerManage::operator=(const ServerManage &src)
{
    if (this != &src)
    {
        this->_host = src._host;
        this->_port = src._port;
        this->_serverAddr = src._serverAddr;
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

void ServerManage::EpollInHandler(void)
{
    // Aceitar nova conexão
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientFd = accept(this->getSocketFd(), (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (clientFd < 0)
    {
        std::cerr << "Erro ao aceitar conexão" << std::endl;
        return;
    }

    // Tornar o socket não-bloqueante
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

    std::cout << "Nova conexão aceita (fd=" << clientFd << " de "
              << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << ")" << std::endl;

    // Criar novo cliente e adicionar ao mapa
    RunTime::getClients().insert(
        std::make_pair(clientFd, Client(clientFd, *this)));

    // Adicionar ao epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = &RunTime::getClient(clientFd);
    epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_ADD, clientFd, &ev);
}
