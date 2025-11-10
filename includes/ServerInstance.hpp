/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInstance.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:38:16 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:38:17 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class ServerInstance
{
	private:
		int					_server_fd;
		struct sockaddr_in	_addr;

	public:

	~ServerInstance();
	ServerInstance();
	ServerInstance(const ServerInstance &src);
	ServerInstance &operator=(const ServerInstance &src);

	int getServerFd() const;
	void setAddr(const struct sockaddr_in& addr);

	void setAddr(int domain, int port, int addr);
	void startSocket(int domain, int socketType);
	void bindSocket();
	void listenSocket();
};
