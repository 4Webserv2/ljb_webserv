/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:36 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/20 16:01:01 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(){
	http_version = "HTTP/1.0";
};

HttpResponse::~HttpResponse(){};

HttpResponse HttpResponse::handleCGI(const HttpRequest &req){
	HttpResponse res;
	std::string path = "./www" + req.uri;
	
	std::string queryString = "";
	size_t queryPos = req.uri.find('?');
	if (queryPos != std::string::npos) {
		queryString = req.uri.substr(queryPos + 1);
		path = "./www" + req.uri.substr(0, queryPos);
	}
	
	// Verificar se o arquivo existe
	if (access(path.c_str(), F_OK) == -1) {
		res.setStatus(404, "Not Found");
		res.setBody("<h1>404 Not Found</h1>", "text/html");
		return res;
	}
	
	int inputPipe[2];
	int outputPipe[2];
	
	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<h1>500 Internal Server Error</h1>", "text/html");
		return res;
	}
	pid_t pid = fork();
	
	if (pid < 0) {
		close(inputPipe[0]);
		close(inputPipe[1]);
		close(outputPipe[0]);
		close(outputPipe[1]);
		
		res.setStatus(500, "Internal Server Error");
		res.setBody("<h1>500 Internal Server Error</h1>", "text/html");
		return res;
	}
	
	if (pid == 0) {
		close(inputPipe[1]);
		dup2(inputPipe[0], STDIN_FILENO);
		close(inputPipe[0]);
		
		close(outputPipe[0]); 
		dup2(outputPipe[1], STDOUT_FILENO);
		close(outputPipe[1]);
		
		setenv("REQUEST_METHOD", req.method.c_str(), 1);
		setenv("QUERY_STRING", queryString.c_str(), 1);
		setenv("CONTENT_LENGTH", req.headers.count("Content-Length") ? req.headers.at("Content-Length").c_str() : "", 1);
		setenv("CONTENT_TYPE", req.headers.count("Content-Type") ? req.headers.at("Content-Type").c_str() : "", 1);
		setenv("SCRIPT_NAME", req.uri.c_str(), 1);
		setenv("SERVER_NAME", req.headers.count("Host") ? req.headers.at("Host").c_str() : "localhost", 1);
		setenv("SERVER_PORT", "8080", 1);
		setenv("SERVER_PROTOCOL", req.version.c_str(), 1);
		setenv("REDIRECT_STATUS", "200", 1);
		
		// Mudar para o diretório do script
		// std::string scriptDir = path.substr(0, path.find_last_of('/'));
		// chdir(scriptDir.c_str());
		
		char* const args[] = {
			const_cast<char*>("python3"),
			const_cast<char*>(path.c_str()),
			NULL
		};
		
		execve("/usr/bin/python3", args, environ);
		exit(1);
	}
	else {
		close(inputPipe[0]);
		close(outputPipe[1]);
		
		if (req.method == "POST" && !req.body.empty()) {
			write(inputPipe[1], req.body.c_str(), req.body.size());
		}
		
		close(inputPipe[1]);
		
		char buffer[4096];
		ssize_t bytesRead;
		std::string cgiOutput;
		
		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
			buffer[bytesRead] = '\0';
			cgiOutput += buffer;
		}
		
		close(outputPipe[0]);
		
		int status;
		waitpid(pid, &status, 0);
		
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			size_t headerEnd = cgiOutput.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				headerEnd = cgiOutput.find("\n\n");
			}
			
			if (headerEnd != std::string::npos) {
				std::string headers = cgiOutput.substr(0, headerEnd);
				std::string body = cgiOutput.substr(headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2));
				
				res.setStatus(200, "OK");
				
				std::istringstream headerStream(headers);
				std::string line;
				while (std::getline(headerStream, line)) {
					if (line.empty() || line == "\r") continue;

					if (!line.empty() && line[line.size() - 1] == '\r') {
						line.erase(line.size() - 1);
					}
					size_t colonPos = line.find(':');
					if (colonPos != std::string::npos) {
						std::string key = line.substr(0, colonPos);
						std::string value = line.substr(colonPos + 1);
						
						while (!value.empty() && isspace(value[0])) {
							value.erase(0, 1);
						}
						if (key == "Status") {
							size_t spacePos = value.find(' ');
							if (spacePos != std::string::npos) {
								res.setStatus(std::atoi(value.substr(0, spacePos).c_str()), 
											value.substr(spacePos + 1));
							}
						} else {
							res.setHeader(key, value);
						}
					}
				}
				
				if (res.headers.count("Content-Type") == 0) {
					res.setBody(body, "text/html");
				} else {
					res.body = body;
					res.headers["Content-Length"] = res.intToString(body.size());
				}
			} else {
				res.setStatus(200, "OK");
				res.setBody(cgiOutput, "text/html");
			}
		} else {
			// Erro na execução do CGI
			res.setStatus(500, "Internal Server Error");
			res.setBody("<h1>500 Internal Server Error</h1><p>Failed to execute CGI script</p>", "text/html");
		}
	}
	
	return res;
}


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
	
	// Verificar se há corpo na requisição
	if (req.body.empty()) {
		res.setStatus(400, "Bad Request");
		res.setBody("<h1>400 Bad Request</h1><p>No data to upload</p>", "text/html");
		return res;
	}
	
	// Criar diretório uploads se não existir
	std::string uploadDir = "./uploads";
	struct stat st;
	if (stat(uploadDir.c_str(), &st) == -1) {
		if (mkdir(uploadDir.c_str(), 0755) != 0) {
			res.setStatus(500, "Internal Server Error");
			res.setBody("<h1>500 Internal Server Error</h1><p>Could not create uploads directory</p>", "text/html");
			return res;
		}
	}
	
	// Gerar nome de arquivo único (timestamp)
	std::time_t now = std::time(0);
	std::ostringstream filename;
	filename << "./uploads/upload_" << now << ".txt";
	
	std::string path = filename.str();

	std::ofstream file(path.c_str());
	if (!file) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<h1>500 Internal Server Error</h1><p>Could not create file</p>", "text/html");
		return res;
	}

	file << req.body;
	file.close();

	res.setStatus(201, "Created");
	std::ostringstream bodyMsg;
	bodyMsg << "<h1>File uploaded successfully!</h1>"
			<< "<p>File saved to: " << path << "</p>"
			<< "<p>Size: " << req.body.size() << " bytes</p>";
	res.setBody(bodyMsg.str(), "text/html");
	return res;
}

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
	std::string extension;
	size_t dotPos = req.uri.find_last_of('.');
	size_t queryPos = req.uri.find('?');
	
	if (dotPos != std::string::npos) {
		if (queryPos != std::string::npos && queryPos > dotPos) {
			extension = req.uri.substr(dotPos, queryPos - dotPos);
		} else {
			extension = req.uri.substr(dotPos);
		}
		if (extension == ".py") {
			return handleCGI(req);
		}
	}
	
	if(req.method == "GET"){
		return handleGet(req);
	} else if(req.method == "POST"){
		return handlePost(req);
	} else if(req.method == "DELETE"){
		return handleDelete(req);
	} else {
		HttpResponse res;
		res.setErrorPage(405);
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

void		HttpResponse::setErrorPage(int code){
	std::string path = "./error_pages/" + intToString(code) + ".html";
	std::ifstream file(path.c_str());
	if(!file){
		setStatus(code, "Error");
		setBody("<h1>"+ intToString(code) + "Error</h1>","text/html");
		return;
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	setStatus(code, "Error");
	setBody(buffer.str(),"text/html");
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
