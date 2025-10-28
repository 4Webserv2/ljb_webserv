/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:45:55 by jbergfel          #+#    #+#             */
/*   Updated: 2025/10/28 20:08:52 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "EpollInstance.hpp"
# include "Client.hpp"
# include "ServerListen.hpp"
# include "ServerConfig.hpp"

class RunTime {
	private:
		static RunTime *_runtime;
		ServerConfig _config;
		EpollInstance _epoll;
		std::map<int, Client> _clients;
		std::vector<ServerListen> _sListeners;
		bool _running;

		RunTime();
		RunTime(int ac, char **av);
		RunTime(const RunTime &src);
		RunTime& operator=(const RunTime &src);

	public:
		~RunTime();
		static void createInstance(int ac, char **av);
		static void destroyInstance(void);

		static void initListeners(void);
		static void initSockets(void);
		static void deleteClient(int client_fd);

		static RunTime &getInstance();
		static EpollInstance &getEpoll();
		static ServerConfig &getServerConfig();
		static std::vector<ServerListen> &getListeners();
		static std::map<int, Client> &getClients();
		static Client &getClient(int client_fd);
};
