#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ServerBlock.hpp"
#include "ServerConfig.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationBlock.hpp"


#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 4096

enum e_status {
	ERROR = -1,
	NO_ERROR = 0
};

#endif

