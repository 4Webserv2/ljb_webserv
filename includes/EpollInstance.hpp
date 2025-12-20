/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:15 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/20 12:20:16 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
// # include "EpollHandler.hpp"

class EpollHandler;

class EpollInstance
{
	private:
		static EpollInstance			*_run;
		int								_epollFd;
		std::map<int, EpollHandler*>	_epollHandlers;
		struct epoll_event				_epollEvents;
		std::vector<int>				_pendingRemovals;
		struct epoll_event				_eventsList[MAX_EVENTS];

		EpollInstance();
		EpollInstance(const EpollInstance &src);
		EpollInstance &operator=(const EpollInstance &src);

	public:
		~EpollInstance();

		static void initEpollRun();
		static void deleteElementFromHandlers(int socketFd);
		static void manipInterestList(int operation, EpollHandler *handler);
		static int	manipEpollWait();
		static void deletePendingRemovals();
		static void replaceHandlerFd(EpollHandler *handler, int newFd, uint32_t newEvents);

		static int getEpollFd();
		static std::map<int, EpollHandler*> &getEpollHandlers();
		static struct epoll_event getEpollEvents();
		static struct epoll_event &getEpollEventsList();
		static struct epoll_event &getElementFromEventsList(int i);

		class CannotInitEpoll : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class CannotManipulate : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};
