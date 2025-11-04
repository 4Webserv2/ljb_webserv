/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInstance.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:29:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/17 17:03:11 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerInstance.hpp"

ServerInstance::~ServerInstance() {}

ServerInstance::ServerInstance() {}

ServerInstance::ServerInstance(const ServerInstance &src)
{
	*this = src;
}

ServerInstance &ServerInstance::operator=(const ServerInstance &src)
{
	if (this != &src)
	{
		this->_addr = src._addr;
		this->_server_fd = src._server_fd;
	}
	return (*this);
}

int ServerInstance::getServerFd() const
{
	return (this->_server_fd);
}

void ServerInstance::setAddr(int domain, int port, int addr)
{
	this->_addr.sin_addr.s_addr = htonl(addr);
	this->_addr.sin_family = domain;
	this->_addr.sin_port = htons(port);
}

void ServerInstance::startSocket(int domain, int socketType)
{
	int serverFd = socket(domain, socketType, 0);

	if (serverFd == -1)
	{
		std::cout << "Failed to initialize server socket" << std::endl;
		return ;
	}
	this->_server_fd = serverFd;
}

void ServerInstance::bindSocket(void)
{
	int bind_status = bind(getServerFd(), (struct sockaddr *)&this->_addr, sizeof(this->_addr));

	if (bind_status == -1)
	{
		std::cout << "Failed to Bind socket" << std::endl;
		return ;
	}
}

void ServerInstance::listenSocket(void)
{
	int listen_status = listen(getServerFd(), MAX_EVENTS);

	if (listen_status == -1)
	{
		std::cout << "Failed to listen Socket" << std::endl;
	}
}
