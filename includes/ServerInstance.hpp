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
