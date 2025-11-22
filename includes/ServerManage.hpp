/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:36:40 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:04:15 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "EpollHandler.hpp"
# include "ServerBlock.hpp"

class ServerManage: public EpollHandler
{
	private:
		unsigned int		_host;
		int					_port;
		struct sockaddr_in	_serverAddr;
		const ServerBlock	&_block;

	public:
		ServerManage(unsigned int host, int port, const ServerBlock &block);
		ServerManage(const ServerManage &src);
		ServerManage &operator=(const ServerManage &src);
		~ServerManage();

		void startSocket(int domain, int type);

		void makeSocket(int domain, int type);
		void bindServer();
		void setServerAddr(int domain);
		void listenSocket();

		unsigned int getHost() const;
		int getPort() const;
		ServerBlock getBlock() const;
};
