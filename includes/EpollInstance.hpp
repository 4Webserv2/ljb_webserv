/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:15 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 09:26:55 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
// # include "EpollHandler.hpp"

class EpollHandler;

class EpollInstance
{
	private:
		static EpollInstance	*_run;
		int	_epollFd;
		std::map<int, EpollHandler*>	_epollHandlers;
		struct epoll_event _epollEvents;
		struct epoll_event _eventsList[MAX_EVENTS];

		EpollInstance();
		EpollInstance(const EpollInstance &src);
		EpollInstance &operator=(const EpollInstance &src);

	public:
		~EpollInstance();

		static int getEpollFd();
		static std::map<int, EpollHandler*> &getEpollHandlers();
		static struct epoll_event getEpollEvents();
		static struct epoll_event &getEpollEventsList();
		static struct epoll_event &getElementFromEventsList(int i);

		static void deleteElementFromHandlers(int socketFd);
		static void initEpoll();
		static void initEpollRun();
		static int manipEpollWait();

		class CannotInitEpoll : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};
