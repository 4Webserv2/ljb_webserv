# pragma once

# include <string>
# include <map>
# include <cstdlib>
# include <cstring>

typedef struct HttpRequest {
	std::string	method;
	std::string	uri;
	std::string	version;
	std::map<std::string, std::string> headers;
	std::string	body;
} HttpRequest;



