#pragma once

# include "EpollHandler.hpp"
# include "ServerBlock.hpp"
# include "Runtime.hpp"

class Client;
class ServerBlock;

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
		ServerBlock getServerBlock(void) const;
		void EpollInHandler(void);

		void startSocket(int domain, int type);
		void makeSocket(int domain, int type);
		void setServerAddr(int domain);
		void reuseAddr();
		void bindServer();
		void updateToNonBlocking();
		void listenSocket();
		int getFd() const;

		unsigned int getHost() const;
		int getPort() const;

		// ALTERADO: Retornar referência constante ao invés de cópia
		const ServerBlock& getBlock() const;
};
