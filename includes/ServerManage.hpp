/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManage.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 23:06:08 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/28 23:06:09 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class ServerManage : public EpollHandler
{
	private:
		unsigned int _host;
		int _port;
		struct sockaddr_in _serverAddr;
		const ServerBlock &_serverBlock;

	public:
		ServerManage(unsigned int host, int port, const ServerBlock &serverBlock);
		ServerManage(const ServerManage &src);
		ServerManage &operator=(const ServerManage &src);
		bool operator==(const ServerManage &other) const;
		~ServerManage(void);

		virtual void handleEpollIn(void);

		unsigned int getHost(void) const;
		int getPort(void) const;
		ServerBlock getServerBlock(void) const;

		void setServerAddr(int socketDomain);
		void createServerSocket(int socketDomain, int socketType);
		void bindServerSocket(void);
		void allowAddrReuse(void);
		void updateToNonBlocking(void);
		void listenServerSocket(void);
		void initServerSocket(int socketDomain, int socketType);

		class CannotInitServerSocket : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class CannotBindServerSocket : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class CannotUpdateServerToNonBlocking : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class CannotSetServerToListen : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class CannotAllowAddrReuse : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};
