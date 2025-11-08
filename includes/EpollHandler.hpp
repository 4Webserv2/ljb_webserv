/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:37:48 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 13:01:19 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Webserv.hpp"

class EpollHandler
{
	private:
		int _socketFd;
		uint32_t _activeEvents;

	public:
		virtual ~EpollHandler();
		EpollHandler(uint32_t activeEvents);
		EpollHandler(int fd, uint32_t activeEvents);

		virtual int EpollEventHandler(struct epoll_event &event);
		virtual void EpollInHandler();
		virtual void EpollOutHandler();

		virtual void setSocketFd(int socketFd);
		virtual int getSocketFd() const;
		virtual uint32_t getActiveEvents() const;
};
