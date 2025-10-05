# pragma once

#include "Webserv.hpp"
#include "ServerConfig.hpp"
#include "LocationBlock.hpp"

class ServerConfig;
class LocationBlock;

typedef struct s_listen
{
	unsigned int    host;
	int             port;
} t_listen;

class ServerBlock
{
	private:
		std::vector<std::string>                _serverNames;
		std::vector<t_listen>                   _listen;
		std::pair<bool, size_t>                 _maxBodySize;
		std::pair<bool, std::string>            _root;
		std::map<std::string, LocationBlock>    _locations;
		std::map<int, std::string>              _errorPages;

	public:
		ServerBlock(ServerConfig &config);
		~ServerBlock();
		
		void printServerBlock();

		void addListens(ServerConfig &config);
		void addServerNames(ServerConfig &config);
		void addMaxBodySize(ServerConfig &config);
		void addRoot(ServerConfig &config);
		void addErrorPages(ServerConfig &config);
		void addLocation(ServerConfig &config);

		//| Getters
		std::vector<std::string> getServerNames() const;
		std::vector<t_listen> getListen() const;
		std::pair<bool, size_t> getMaxBodySize() const;
		std::pair<bool, std::string> getRoot() const;
		std::map<int, std::string> getErrorPages() const;
		std::map<std::string, LocationBlock> getLocations() const;
};