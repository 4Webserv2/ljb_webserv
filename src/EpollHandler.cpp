/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 07:47:56 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/28 09:09:36 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollHandler.hpp"
#include "../includes/EpollInstance.hpp"

EpollHandler::~EpollHandler(void) {}

EpollHandler::EpollHandler(uint32_t activeEvents)
{
	this->_socketFd = -1;
	this->_activeEvents = activeEvents;
	this->_eventsTimeout = 30;
}

EpollHandler::EpollHandler(int socketFd, uint32_t activeEvents, int eventsTimeout)
{
	this->_socketFd = socketFd;
	this->_activeEvents = activeEvents;
	this->_eventsTimeout = eventsTimeout;
}

void EpollHandler::EpollInHandler(void)
{}

void EpollHandler::EpollOutHandler(void)
{}

int EpollHandler::EpollEventHandler(struct epoll_event &event)
{
	if (event.events & (EPOLLIN | EPOLLRDHUP))
		this->EpollInHandler();
	else if (event.events & EPOLLOUT)
		this->EpollOutHandler();
	else
		return (-1);
	return (0);
}

void EpollHandler::deleteHandler(void)
{
	if (this->_socketFd != -1)
	{
		close(this->_socketFd);
		this->_socketFd = -1;
	}
}

void EpollHandler::handleTimeout(void)
{
	if (this->_eventsTimeout < 0)
		return;
	time_t currentTime = time(NULL);
	if (currentTime - this->_epollTime > this->_eventsTimeout)
		EpollInstance::deleteElementFromHandlers(this->_socketFd);
}

void EpollHandler::setSocketFd(int socketFd)
{
	this->_socketFd = socketFd;
}

void EpollHandler::setEventsTimeout(int eventsTimeout)
{
	this->_eventsTimeout = eventsTimeout;
}

int EpollHandler::getSocketFd(void) const
{
	return (this->_socketFd);
}

uint32_t EpollHandler::getActiveEvents(void) const
{
	return (this->_activeEvents);
}

int EpollHandler::getEventsTimeout(void) const
{
	return (this->_eventsTimeout);
}
