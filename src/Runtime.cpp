/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:32:41 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/28 00:19:00 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Runtime.hpp"
#include "../includes/ServerManage.hpp"

RunTime *RunTime::_runtime = NULL;

RunTime::~RunTime(void) {}

RunTime::RunTime(void) {}

RunTime::RunTime(int ac, char **av): _config(ac, av) {}

RunTime::RunTime(const RunTime &src)
{
	*this = src;
}

RunTime &RunTime::operator=(const RunTime &src)
{
	if (this != &src)
		this->_config = src._config;
	return (*this);
}

int RunTime::createRuntime(int ac, char **av)
{
	if (_runtime == NULL)
	{
		_runtime = new RunTime(ac, av);
		EpollInstance::initEpollRun();
		_runtime->initListeners();
		_runtime->initSockets(AF_INET, SOCK_STREAM);
		return (0);
	}
	return (1);
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
				_runtime->_sListeners.push_back(ServerManage(listens[j].host, listens[j].port, _runtime->_config.getServerBlocks()[i]));
		}
	}
}

void RunTime::initSockets(int domain, int type)
{
	for (unsigned int i = 0; i < _runtime->_sListeners.size(); i++)
	{
		_runtime->_sListeners[i].startSocket(domain, type);
		//EpollInstance::manipInterestList(EPOLL_CTL_ADD, &_runtime->_sListeners[i]);
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

std::vector<ServerManage> &RunTime::getListeners(void)
{
	return (_runtime->_sListeners);
}

std::map<int, Client> &RunTime::getClients(void)
{
	return (_runtime->_clients);
}

Client &RunTime::getClient(int client_fd)
{
	std::map<int, Client>::iterator clients = _runtime->_clients.find(client_fd);
	if (clients == _runtime->_clients.end())
	{
		std::cout << "Cannot find client!" << std::endl;
		return (clients->second);
	}
	return (clients->second);
}

ServerManage &RunTime::getElementInServerList(int serverSocketFd)
{
	for (size_t i = 0; i < _runtime->_sListeners.size(); i++)
	{
		if (_runtime->_sListeners[i].getSocketFd() == serverSocketFd)
			return (_runtime->_sListeners[i]);
	}
	return (_runtime->_sListeners[0]);;
}
