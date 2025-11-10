/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollInstance.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:15 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:39:16 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
