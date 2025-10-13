# include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(){};

HttpResponse::~HttpResponse(){};

HttpResponse	HttpResponse::handleGet(const HttpRequest &req){
	HttpResponse res;
	std::string path = "./www" + req.uri; // root simplificada

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) {
		res.setStatus(404, "Not Found");
		res.setBody("<h1>404 Not Found</h1>", "text/html");
		return res;
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	res.setStatus(200, "OK");
	res.setBody(buffer.str(), "text/html"); // simplificado (todo: detectar mime-type)
	return res;
};

HttpResponse	HttpResponse::handlePost(const HttpRequest &req){
	HttpResponse res;
	std::string path = "./uploads/upload.txt";

	std::ofstream file(path.c_str());
	if (!file) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<h1>500 Internal Server Error</h1>", "text/html");
		return res;
	}

	file << req.body;
	file.close();

	res.setStatus(201, "Created");
	res.setBody("<h1>File uploaded successfully!</h1>", "text/html");
	return res;
};

HttpResponse	HttpResponse::handleDelete(const HttpRequest &req){
		HttpResponse res;
		std::string path = "./www" + req.uri;

		if (std::remove(path.c_str()) == 0) {
			res.setStatus(200, "OK");
			res.setBody("<h1>File deleted successfully</h1>", "text/html");
		} else {
			res.setStatus(404, "Not Found");
			res.setBody("<h1>404 Not Found</h1>", "text/html");
		}
		return res;
};

HttpResponse	HttpResponse::dispatchRequest(const HttpRequest &req){
	if(req.method == "GET"){
		return handleGet(req);
	} else if(req.method == "POST"){
		return handlePost(req);
	} else if(req.method == "DELETE"){
		return handleDelete(req);
	}else{
		HttpResponse res;
		res.setStatus(405, "Method not allowed");
		res.setHeader("Content-type", "text/html");
		res.setBody("<h1>405: Method not allowed</h1>","text/plain");
		return res;
	}
}

void		HttpResponse::setStatus(int code, const std::string &message){
	this->status_code = code;
	this->status_message = message;
};

void		HttpResponse::setHeader(const std::string &key, const std::string &value){
	this->headers[key] = value;
};

void		HttpResponse::setBody(const std::string &b, const std::string &contentType){
	body = b;
	this->headers["Content-Type"] = contentType;
	this->headers["Content-Length"] = intToString(body.size());
}

std::string	HttpResponse::toString() const{
	std::ostringstream response;

	// Status line
	response << http_version << " "
				<< status_code << " "
				<< status_message << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
			it != headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}

	// Linha em branco
	response << "\r\n";

	// Corpo
	response << body;

	return response.str();
}

std::string	HttpResponse::intToString(int n) const{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}