/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:32:41 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/10 18:33:09 by jbergfel         ###   ########.fr       */
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

void RunTime::createRuntime(int ac, char **av)
{
	if (_runtime == NULL)
	{
		_runtime = new RunTime(ac, av);
		//init listners
		//init sockets
	}
}

void RunTime::initListeners(void)
{
	std::cout << "Loading server listeners..." << std::endl;
	std::set<std::pair<unsigned int, int> > uniqueListens;
	for (size_t i = 0; i < _runtime->_config.getServerBlocks().size(); i++)
	{
		std::cout << "Loading server block " << i + 1 << "..." << std::endl;
		std::vector<t_listen> listens = _runtime->_config.getServerBlocks()[i].getListen();

		for (size_t j = 0; j < listens.size(); j++)
		{
			std::pair<unsigned int, int> key(listens[j].host, listens[j].port);
			if (uniqueListens.insert(key).second)
				_runtime->_sListeners.push_back(ServerListen(listens[j].host, listens[j].port, _runtime->_config.getServerBlocks()[i]));
		}
	}
}

void RunTime::destroyRuntime(void)
{
	if (_runtime != NULL)
	{
		delete _runtime;
		_runtime = NULL;
	}
}


RunTime &RunTime::getRuntime(void)
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
