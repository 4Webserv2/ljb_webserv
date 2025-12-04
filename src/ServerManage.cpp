/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 20:58:51 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/04 10:33:22 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerManage.hpp"
#include "../includes/Client.hpp"
#include "../includes/Runtime.hpp"
#include "../includes/EpollInstance.hpp"


int make_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

ServerManage::~ServerManage(){}

ServerManage::ServerManage(unsigned int host, int port, const ServerBlock &block)
	: EpollHandler(EPOLLIN | EPOLLRDHUP), _host(host), _port(port), _block(block)
{}

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
	try {
		this->makeSocket(domain, type);
		this->setServerAddr(domain);
		this->reuseAddr();
		this->bindServer();  // Pode lançar exceção agora
		this->updateToNonBlocking();
		this->listenSocket();
		
		std::cout << "[OK] Server started on port " << this->getPort() << std::endl;
	}
	catch (const std::exception &e) {
		// Fechar socket se algo falhar
		if (this->getSocketFd() != -1) {
			close(this->getSocketFd());
			this->setSocketFd(-1);
		}
		
		// Re-lançar a exceção
		throw;
	}
}

void	ServerManage::makeSocket(int domain, int type)
{
	int	initSocket = socket(domain, type, 0);
	std::cout << "socket()" << this->getSocketFd() << std::endl;
	if (initSocket == -1)
		throw(std::runtime_error("Cannot init Server socket!"));
	this->setSocketFd(initSocket);
}

void ServerManage::setServerAddr(int domain)
{
	this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
	this->_serverAddr.sin_family = domain;
	this->_serverAddr.sin_port = htons(this->getPort());
}

void ServerManage::reuseAddr()
{
	int option = 1;

	if (setsockopt(this->getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
		throw(std::runtime_error("Cannot reuse addr"));
}

void ServerManage::bindServer(void)
{
	int initBind = bind(this->getSocketFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr));
	if (initBind == -1)
	{
		// Mensagem de erro mais detalhada
		std::cerr << "[ERROR] Cannot bind Server socket on port " << this->getPort() 
				<< ": " << strerror(errno) << std::endl;
		
		// IMPORTANTE: Lançar exceção ao invés de apenas retornar
		throw std::runtime_error("Failed to bind socket on port " + 
								std::string(1, '0' + this->getPort()));
	}
	
	std::cout << "[OK] Socket bound successfully on port " << this->getPort() << std::endl;
}

void ServerManage::updateToNonBlocking(void)
{
	try
	{
		std::cout << "nonblocking()" << this->getSocketFd() << std::endl;
		make_nonblocking(this->getSocketFd());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}

void ServerManage::listenSocket(void)
{
	std::cout << "listen()" << this->getSocketFd() << std::endl;
	int initListen = listen(this->getSocketFd(), MAX_EVENTS);
	if (initListen == -1)
		throw(std::runtime_error("Cannot listen to Server socket!"));
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
	std::cout << "New connection incoming on server socket FD: " << this->getSocketFd() << std::endl;
	while (true)
	{
		struct sockaddr_in clientSocketAddr;
		socklen_t clientSocketLength = sizeof(clientSocketAddr);
		int clientFd = accept(this->getSocketFd(), (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
		}
		else
		{
			try
			{
				make_nonblocking(clientFd);
				int flag = 1;
				if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0)
					std::cerr << "[Warning] Failed to set TCP_NODELAY on client socket" << std::endl;

				RunTime::getClients().insert(std::make_pair(clientFd, Client(clientFd, RunTime::getElementInServerList(this->getSocketFd()))));
				std::cout << "inseriu novo client no map." << std::endl;
				EpollInstance::manipInterestList(EPOLL_CTL_ADD, &RunTime::getClient(clientFd));
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << std::endl;
				close(clientFd);
			}
		}
	}
}
