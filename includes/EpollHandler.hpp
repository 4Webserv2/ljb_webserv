/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:37:48 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/29 09:55:11 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Webserv.hpp"

class EpollInstance;

class EpollHandler
{
	private:
		int			_socketFd;
		int			_eventsTimeout;
		uint32_t	_activeEvents;
		time_t		_epollTime;

	public:
		virtual ~EpollHandler();
		EpollHandler(uint32_t activeEvents);
		EpollHandler(int fd, uint32_t activeEvents, int eventsTimeout);

		int		EpollEventHandler(struct epoll_event &event);
		void	handleTimeout();

		virtual void	EpollOutHandler() {};
		virtual void	deleteHandler() {};
		virtual void	EpollInHandler() {};

		virtual void		setSocketFd(int socketFd);
		virtual void		setEventsTimeout(int eventsTimeout);
		virtual void		setActiveEvents(uint32_t event);
		virtual int			getSocketFd() const;
		virtual uint32_t	getActiveEvents() const;
		virtual int			getEventsTimeout() const;
};
