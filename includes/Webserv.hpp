/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 23:06:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/28 23:06:19 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 4096

#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cerrno>
#include <exception>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cctype>
#include <set>
#include <csignal>
#include <sys/wait.h>
#include <dirent.h>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerBlock.hpp"
#include "ServerConfig.hpp"
#include "EpollHandler.hpp"
#include "EpollInstance.hpp"
#include "LocationBlock.hpp"
#include "ServerManage.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "StdLogHandler.hpp"
#include "CompositeLogHandler.hpp"
#include "CgiHandler.hpp"
#include "RunTime.hpp"
#include "FileLogHandler.hpp"

typedef struct s_logEvent
{
	enum LogLevel level;
	std::string message;
} t_logEvent;



enum clientBufferState
{
	READING_HEADER = 9,
	READING_BODY = 10,
	WAITING_CGI = 11,
	COMPLETE = 12,
};

enum cgiBufferState
{
	IN_PROGRESS = 25,
	FAILED = 26,
	COMPLETED = 27
};

void setNonBlocking(int sockfd);
std::string intToString(int n);
std::string extractUriWithoutQuery(const std::string &uri);
std::string extractAndDecodeUri(const std::string &uri);
std::string extractQueryFromUri(const std::string &uri);
std::string extractUriPathInfo(const std::string &uri, const LocationBlock &location);
void errorAndCerr(const std::string &msg);
