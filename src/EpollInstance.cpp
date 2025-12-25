/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 20:46:14 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/24 15:36:44 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/EpollInstance.hpp"
#include "../includes/EpollHandler.hpp"
#include "../includes/Logger.hpp"
#include "../includes/StringUtils.hpp"

EpollInstance *EpollInstance::_run = NULL;

EpollInstance::EpollInstance() {}

EpollInstance::~EpollInstance()
{
	for (std::map<int, EpollHandler *>::iterator it = _epollHandlers.begin(); it != _epollHandlers.end(); ++it)
	{
		delete it->second;
	}
	_epollHandlers.clear();

	if (_epollFd != -1)
	{
		close(_epollFd);
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

void EpollInstance::initEpollRun(void)
{
	if (_run == NULL)
	{
		_run = new EpollInstance();
		_run->_epollFd = epoll_create(1);
		if (_run->_epollFd == -1)
			throw(std::runtime_error("Cannot Init Epoll!"));
		Logger::info("Epoll created!");
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

void EpollInstance::manipInterestList(int operation, EpollHandler *handler)
{
	if (_run == NULL)
		throw std::runtime_error("EpollInstance is not initialized.");

	if (!handler)
		throw std::invalid_argument("manipInterestList: handler is null");

	int socketFd = handler->getSocketFd();

	if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD)
		throw(EpollInstance::CannotManipulate());

	struct epoll_event data;
	data.events = handler->getActiveEvents();
	Logger::info("Inside manipInterest. FD: " + StringUtils::intToString(socketFd));
	data.data.ptr = handler;
	if (operation == EPOLL_CTL_DEL)
	{
		if (std::find(_run->_pendingRemovals.begin(), _run->_pendingRemovals.end(), socketFd) != _run->_pendingRemovals.end())
		{
			Logger::debug("manipInterestList: fd=" + StringUtils::intToString(socketFd) + " already pending removal");
			return;
		}
		if (_run->_epollHandlers.find(socketFd) == _run->_epollHandlers.end())
		{
			Logger::debug("manipInterestList: skip EPOLL_CTL_DEL for unknown fd=" + StringUtils::intToString(socketFd));
			return;
		}
		_run->_pendingRemovals.push_back(socketFd);
	}
	else if (operation == EPOLL_CTL_ADD || operation == EPOLL_CTL_MOD)
	{
		_run->_epollHandlers[socketFd] = handler;
	}
	if (epoll_ctl(_run->_epollFd, operation, socketFd, &data) == -1)
	{
		if (errno == ENOENT || errno == EBADF)
		{
			Logger::debug("epoll_ctl returned ENOENT/EBADF for fd=" + StringUtils::intToString(socketFd) + " op=" + StringUtils::intToString(operation) + " (ignoring)");
			if (operation == EPOLL_CTL_DEL)
			{
				std::map<int, EpollHandler *>::iterator it = _run->_epollHandlers.find(socketFd);
				if (it != _run->_epollHandlers.end())
					_run->_epollHandlers.erase(it);
			}
			return;
		}
		StringUtils::errorAndCerr("epoll_ctl failed: op=" + StringUtils::intToString(operation) + " fd=" + StringUtils::intToString(socketFd) + " errno=" + StringUtils::intToString(errno) + " (" + std::string(strerror(errno)) + ")");
		throw(EpollInstance::CannotManipulate());
	}
	Logger::info("Epoll Manipulation ompleted successfully!");
}

void EpollInstance::replaceHandlerFd(EpollHandler *handler, int newFd, uint32_t newEvents)
{
	if (_run == NULL)
		throw std::runtime_error("EpollInstance is not initialized.");
	if (!handler)
		throw std::invalid_argument("replaceHandlerFd: handler is null");

	int oldFd = handler->getSocketFd();
	int epfd = _run->_epollFd;
	if (epfd == -1)
		throw std::runtime_error("replaceHandlerFd: invalid epoll fd");
	if (newEvents == 0)
		newEvents = handler->getActiveEvents();

	if (oldFd == newFd && oldFd != -1)
	{
		struct epoll_event ev;
		ev.events = newEvents;
		ev.data.ptr = handler;
		if (epoll_ctl(epfd, EPOLL_CTL_MOD, oldFd, &ev) == -1)
		{
			std::cerr << "replaceHandlerFd: EPOLL_CTL_MOD failed: fd=" << oldFd << " errno=" << errno << " (" << strerror(errno) << ")\n";
		}
		handler->setActiveEvents(newEvents);
		_run->_epollHandlers[oldFd] = handler;
		return;
	}

	if (oldFd != -1)
	{
		if (epoll_ctl(epfd, EPOLL_CTL_DEL, oldFd, NULL) == -1)
		{
			if (errno != ENOENT && errno != EBADF)
				std::cerr << "replaceHandlerFd: EPOLL_CTL_DEL failed for oldFd=" << oldFd << " errno=" << errno << " (" << strerror(errno) << ")\n";
			else
				Logger::debug("replaceHandlerFd: EPOLL_CTL_DEL returned ENOENT/EBADF for oldFd=" + StringUtils::intToString(oldFd) + " (continuing)");
		}

		std::map<int, EpollHandler *>::iterator it = _run->_epollHandlers.find(oldFd);
		if (it != _run->_epollHandlers.end())
			_run->_epollHandlers.erase(it);
	}

	struct epoll_event ev;
	ev.events = newEvents;
	ev.data.ptr = handler;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, newFd, &ev) == -1)
	{
		std::cerr << "replaceHandlerFd: EPOLL_CTL_ADD failed for newFd=" << newFd << " errno=" << errno << " (" << strerror(errno) << ")\n";
		handler->setSocketFd(-1);
	}
	handler->setSocketFd(newFd);
	handler->setActiveEvents(newEvents);
	_run->_epollHandlers[newFd] = handler;
}

int EpollInstance::manipEpollWait(void)
{
	if (_run == NULL)
		throw std::runtime_error("EpollInstance is not initialized.");
	int numberOfReadyFds = 0;
	numberOfReadyFds = epoll_wait(_run->_epollFd, _run->_eventsList, MAX_EVENTS, -1);
	return (numberOfReadyFds);
}

void EpollInstance::deletePendingRemovals()
{
	if (_run == NULL)
	{
		throw std::runtime_error("EpollInstance is not initialized.");
	}
	for (size_t i = 0; i < _run->_pendingRemovals.size(); i++)
	{
		int fd = _run->_pendingRemovals[i];

		std::map<int, EpollHandler *>::iterator it =
			_run->_epollHandlers.find(fd);

		if (it != _run->_epollHandlers.end())
		{
			delete it->second;
			_run->_epollHandlers.erase(it);
		}
	}
	_run->_pendingRemovals.clear();
}

int EpollInstance::getEpollFd(void)
{
	return (_run->_epollFd);
}

std::map<int, EpollHandler *> &EpollInstance::getEpollHandlers(void)
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

const char *EpollInstance::CannotManipulate::what() const throw()
{
	static const char msg[] = "Failed to manipulate epoll instance with epoll_ctl().";
	Logger::error(msg);
	return msg;
}
