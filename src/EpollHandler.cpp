/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 07:47:56 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/31 13:48:32 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollHandler.hpp"

EpollHandler::~EpollHandler(void) {}

EpollHandler::EpollHandler(uint32_t activeEvents)
{
	this->_socketFd = -1;
	this->_activeEvents = activeEvents;
}

EpollHandler::EpollHandler(int socketFd, uint32_t activeEvents)
{
	this->_socketFd = socketFd;
	this->_activeEvents = activeEvents;
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

void EpollHandler::setSocketFd(int socketFd)
{
	this->_socketFd = socketFd;
}

int EpollHandler::getSocketFd(void)
{
	return (this->_socketFd);
}

uint32_t EpollHandler::getActiveEvents(void)
{
	return (this->_activeEvents);
}
