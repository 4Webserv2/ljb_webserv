#pragma once

# include "Webserv.hpp"

class EpollInstance
{
	private:
		int	_epollFd;
		struct epoll_event _epollEvents;
		struct epoll_event _eventsList[MAX_EVENTS];

	public:
		~EpollInstance();
		EpollInstance();
		EpollInstance(const EpollInstance &src);
		EpollInstance &operator=(const EpollInstance &src);

		int getEpollFd() const;
		struct epoll_event getEpollEvents() const;
		struct epoll_event &getEpollEventsList();
		struct epoll_event &getElementFromEventsList(int i);

		void initEpoll();

		class CannotInitEpoll : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};
