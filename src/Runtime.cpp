/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:32:41 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/28 20:07:15 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Runtime.hpp"


RunTime::~RunTime(void) {}

RunTime::RunTime(void) {}

RunTime::RunTime(int ac, char **av): _config(ac, av), _epoll() {}

RunTime::RunTime(const RunTime &src)
{
	*this = src;
}

RunTime &RunTime::operator=(const RunTime &src)
{
	if (this != &src)
	{
		this->_epoll = src._epoll;
		this->_config = src._config;
	}
	return (*this);
}

void RunTime::createInstance(int ac, char **av)
{
	if (_runtime == NULL)
	{
		_runtime = new RunTime(ac, av);
		//init listners
		//init sockets
	}
}

void RunTime::destroyInstance(void)
{
	if (_runtime != NULL)
	{
		delete _runtime;
		_runtime = NULL;
	}
}


RunTime &RunTime::getInstance(void)
{
	return (*_runtime);
}

ServerConfig &RunTime::getServerConfig(void)
{
	return (_runtime->_config);
}

EpollInstance &RunTime::getEpoll(void)
{
	return (_runtime->_epoll);
}

std::vector<ServerListen> &RunTime::getListeners(void)
{
	return (_runtime->_sListeners);
}

std::map<int, Client> &RunTime::getClients(void)
{
	return (_runtime->_clients);
}

Client &RunTime::getClient(int client_fd)
{
	
}
