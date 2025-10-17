/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:46:14 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/17 16:45:43 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollInstance.hpp"

EpollInstance::~EpollInstance()
{
	close(this->_epollFd);
}

EpollInstance::EpollInstance()
{
	try
	{
		std::cout << "Try to create new Epoll" << std::endl;
		initEpoll();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

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


void EpollInstance::initEpoll(void)
{
	this->_epollFd = epoll_create1(1);
	if (this->_epollFd == -1)
	throw(EpollInstance::CannotInitEpoll());
}

int EpollInstance::getEpollFd(void) const
{
	return (this->_epollFd);
}

struct epoll_event EpollInstance::getEpollEvents() const
{
	return (this->_epollEvents);
}

struct epoll_event &EpollInstance::getEpollEventsList()
{
	return (*this->_eventsList);
}

struct epoll_event &EpollInstance::getElementFromEventsList(int i)
{
	return (this->_eventsList[i]);
}

