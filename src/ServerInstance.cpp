/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInstance.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:29:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/13 20:45:14 by jbergfel         ###   ########.fr       */
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

void ServerInstance::setAddr(const struct sockaddr_in& addr)
{
	this->_addr = addr;
}
