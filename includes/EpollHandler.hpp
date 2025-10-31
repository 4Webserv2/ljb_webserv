/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:37:48 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/31 13:46:55 by jbergfel         ###   ########.fr       */
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
		~EpollHandler();
		EpollHandler(uint32_t activeEvents);
		EpollHandler(int fd, uint32_t activeEvents);

		int EpollEventHandler(struct epoll_event &event);
		void EpollInHandler();
		void EpollOutHandler();

		//Getter e Setter

		void setSocketFd(int socketFd);
		int getSocketFd();
		uint32_t getActiveEvents();
};
