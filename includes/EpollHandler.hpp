/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:37:48 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 09:34:07 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Webserv.hpp"

class EpollInstance;

class EpollHandler
{
	private:
		int			_socketFd;
		uint32_t	_activeEvents;
		int			_eventsTimeout;
		time_t		_epollTime;

	public:
		virtual ~EpollHandler();
		EpollHandler(uint32_t activeEvents);
		EpollHandler(int fd, uint32_t activeEvents, int eventsTimeout);

		virtual int EpollEventHandler(struct epoll_event &event);
		virtual void EpollInHandler();
		virtual void EpollOutHandler();
		virtual void deleteHandler();

		virtual void setSocketFd(int socketFd);
		virtual void setEventsTimeout(int eventsTimeout);
		virtual void handleTimeout();

		virtual int			getSocketFd() const;
		virtual uint32_t	getActiveEvents() const;
		virtual int			getEventsTimeout() const;
};
