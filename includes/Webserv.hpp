/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:50:29 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/23 20:54:57 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <cerrno>
# include <exception>
# include <algorithm>
# include <vector>
# include <map>
# include <string>
# include <utility>
# include <cstddef>
# include <cstdlib>
# include <ctime>
# include <fstream>
# include <sstream>
# include <cctype>
# include <set>
# include <csignal>
# include <sys/types.h>
# include <sys/wait.h>
# include <clocale>
# include <cstring>



// # include "HttpParser.hpp"
// # include "HttpResponse.hpp"
// # include "Client.hpp"
// # include "EpollInstance.hpp"
// # include "ConfigFile.hpp"
// # include "Runtime.hpp"
// # include "ServerInstance.hpp"
// # include "ServerBlock.hpp"
// # include "ServerConfig.hpp"
// # include "LocationBlock.hpp"

# define PORT 8080
# define BACKLOG 10
# define MAX_EVENTS 10
# define MAX_BUFFER_SIZE 4096

enum e_status {
	E_ERROR = -1,
	NO_ERROR = 0
};

enum clientBufferState {
    READING_HEADER = 9, //Lendo o header da request ainda
    READING_BODY = 10, //Lendo o conteudo da request ainda
    WAITING_CGI = 11, //Aguardando resposta do CGI
    COMPLETE = 12, //Ja lemos todo o conteudo da request
};

enum cgiBufferState {
    IN_PROGRESS = 25,
    FAILED = 26,
    COMPLETED = 27
};
int test();

#endif

