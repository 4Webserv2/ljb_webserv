/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:46:14 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 22:48:04 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollInstance.hpp"
#include "../includes/EpollHandler.hpp"

EpollInstance *EpollInstance::_run = NULL;

EpollInstance::~EpollInstance()
{
	close(this->_epollFd);
}

EpollInstance::EpollInstance()
{}

EpollInstance::EpollInstance(const EpollInstance &src)
{
	*this = src;
}

EpollInstance &EpollInstance::operator=(const EpollInstance &src)
{
	if (this != &src)
	{
		this->_epollFd = src._epollFd;
		this->_epollEvents = src._epollEvents;
		for (size_t i = 0; i < MAX_EVENTS; i++)
		{
			this->_eventsList[i].data.fd = src._eventsList[i].data.fd;
			this->_eventsList[i].events = src._eventsList[i].events;
		}
	}
	return (*this);
}

void EpollInstance::initEpollRun(void)
{
	if (_run == NULL)
	{
		_run = new EpollInstance();
		_run->_epollFd = epoll_create(1);
		if (_run->_epollFd == -1)
			throw (std::runtime_error("Cannot Init Epoll!"));
		std::cout << "Epoll created!" << std::endl;
	}
}

void EpollInstance::deleteElementFromHandlers(int socketFd)
{
	std::map<int, EpollHandler *>::iterator it = _run->_epollHandlers.find(socketFd);
	if (it != _run->_epollHandlers.end())
	{
		it->second->deleteHandler();
		_run->_epollHandlers.erase(it);
	}
}

int EpollInstance::manipEpollWait(void)
{
	if (_run == NULL)
	{
		throw std::runtime_error("EpollInstance is not initialized.");
	}

	int numberOfReadyFds = 0;
	numberOfReadyFds = epoll_wait(_run->_epollFd, _run->_eventsList, MAX_EVENTS, -1);
	return (numberOfReadyFds);
}

int EpollInstance::getEpollFd(void)
{
	return (_run->_epollFd);
}

std::map<int, EpollHandler*> &EpollInstance::getEpollHandlers(void)
{
	return (_run->_epollHandlers);
}

struct epoll_event EpollInstance::getEpollEvents()
{
	return (_run->_epollEvents);
}

struct epoll_event &EpollInstance::getEpollEventsList()
{
	return (*_run->_eventsList);
}

struct epoll_event &EpollInstance::getElementFromEventsList(int i)
{
	return (_run->_eventsList[i]);
}

