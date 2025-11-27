/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:45:55 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/27 09:55:05 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "EpollInstance.hpp"
# include "Client.hpp"
# include "ServerConfig.hpp"

class ServerManage;

class RunTime {
	private:
		static RunTime *_runtime;
		ServerConfig _config;
		std::map<int, Client> _clients;
		std::vector<ServerManage> _sListeners;
		bool _running;

		RunTime();
		RunTime(int ac, char **av);
		RunTime(const RunTime &src);
		RunTime& operator=(const RunTime &src);

	public:
		~RunTime();
		static void createRuntime(int ac, char **av);
		static void destroyRuntime(void);

		static void initListeners(void);
		static void initSockets(int domain, int type);
		static void deleteClient(int client_fd);

		static RunTime &getRuntime();
		static ServerConfig &getServerConfig();
		static std::vector<ServerManage> &getListeners();
		static std::map<int, Client> &getClients();
		static Client &getClient(int client_fd);
		static ServerManage &getElementInServerList(int serverSocketFd);
};
