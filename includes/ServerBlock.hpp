/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:38:08 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/04 10:24:39 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# pragma once

# include "Webserv.hpp"
# include "LocationBlock.hpp"

class ServerConfig;
class LocationBlock;

typedef struct s_listen
{
	unsigned int	host;
	int				port;
} t_listen;

class ServerBlock
{
	private:
		ServerConfig&							_config;
		std::vector<std::string>                _serverNames;
		std::vector<t_listen>                   _listen;
		std::pair<bool, size_t>                 _maxBodySize;
		std::pair<bool, std::string>            _root;
		std::map<std::string, LocationBlock>    _locations;
		std::map<int, std::string>              _errorPages;
		unsigned int							_cgiTimeout;

	public:
		ServerBlock(ServerConfig &config);
		ServerBlock(const ServerBlock &src);
		ServerBlock &operator=(const ServerBlock &src); 
		~ServerBlock();

		void printServerBlock();

		bool isUriValid(const std::string uri);
		bool isLocationValid(const std::string uri, const std::string method);

		void addListens();
		void addServerNames();
		void addMaxBodySize();
		void addRoot();
		void addErrorPages();
		void addLocation();
		unsigned int getCgiTimeout() const;
		void setCgiTimeout(unsigned int timeout);

		//| Getters
		std::vector<std::string> getServerNames() const;
		std::vector<t_listen> getListen() const;
		std::pair<bool, size_t> getMaxBodySize() const;
		std::pair<bool, std::string> getRoot() const;
		std::map<int, std::string> getErrorPages() const;
		std::map<std::string, LocationBlock> getLocations() const;
};
