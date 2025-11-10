/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerListen.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:36:40 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:12:46 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "EpollHandler.hpp"
# include "ServerBlock.hpp"

class ServerListen: public EpollHandler
{
	private:
		unsigned int		_host;
		int					_port;
		struct sockaddr_in	_serverAddr;
		const ServerBlock	&_block;

	public:
		ServerListen(unsigned int host, int port, const ServerBlock &block);
		ServerListen(const ServerListen &src);
		ServerListen &operator=(const ServerListen &src);
		~ServerListen();

		void startSocket(int domain, int type);

		void makeSocket(int domain, int type);
		void bindServer();
		void setServerAddr(int domain);
		void listenSocket();

		unsigned int getHost() const;
		int getPort() const;
		ServerBlock getBlock() const;

};
