/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:46:14 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/29 07:50:43 by jbergfel         ###   ########.fr       */
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

void EpollInstance::manipInterestList(int operation, EpollHandler *handler) {
	if (_run == NULL)
		throw std::runtime_error("EpollInstance is not initialized.");

	if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD)
		throw(EpollInstance::CannotManipulate());

	struct epoll_event data;
	data.events = handler->getActiveEvents();
	std::cout << "Dentro do manipInterest. FD: " << handler->getSocketFd() << std::endl;
	data.data.ptr = handler;
	if (operation == EPOLL_CTL_ADD)
		_run->_epollHandlers[handler->getSocketFd()] = handler;

	if (epoll_ctl(_run->_epollFd, operation, handler->getSocketFd(), &data) == -1)
	{
		std::cerr << "epoll_ctl failed: op=" << operation << " fd=" << handler->getSocketFd() << " errno=" << errno << " (" << strerror(errno) << ")\n";
		throw(EpollInstance::CannotManipulate());
	}
	std::cout << "Manipulação feita com sucesso!" << std::endl;
}

int EpollInstance::manipEpollWait(void)
{
	if (_run == NULL)
		throw std::runtime_error("EpollInstance is not initialized.");
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
	if (_run == NULL)
		throw(std::runtime_error("Erro"));
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
	if (_run == NULL)
		throw(std::runtime_error("Epoll unitialized!"));
	return (_run->_eventsList[i]);
}

const char * EpollInstance::CannotManipulate::what() const throw()
{
	return ("Error: error in controling the epoll instance with epoll_ctl().");
}
