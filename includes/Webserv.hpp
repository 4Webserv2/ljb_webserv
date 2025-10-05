/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:50:29 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/05 20:51:00 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <sys/socket.h>
# include <iostream>
# include <netinet/in.h>
# include <cstdlib>
# include <unistd.h>
# include <arpa/inet.h>
# include <vector>
# include <map>
# include <string>
# include <iostream>
# include <fstream>
# include <sstream>

# include "HttpParser.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Client.hpp"
# include "EpollInstance.hpp"
# include "ConfigFile.hpp"
# include "Runtime.hpp"
# include "ServerInstance.hpp"
# include "ServerBlock.hpp"

# define PORT 8080
# define BACKLOG 10
# define BUFFER_SIZE 4096

enum e_erros {
	ERROR = -1,
	NO_ERROR = 0
};

#endif

