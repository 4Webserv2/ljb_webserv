/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:36 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/02 11:24:08 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse()
{
    http_version = "HTTP/1.1";
    status_code = 200;
    status_message = "OK";
    _errorPages = NULL;
    _rootPath = "./www";
}

HttpResponse::~HttpResponse(){}

void HttpResponse::setErrorPageConfig(const std::map<int, std::string> *errorPages, const std::string &rootPath)
{
    this->_errorPages = errorPages;
    this->_rootPath = rootPath;
}

HttpResponse HttpResponse::handleCGI(const HttpRequest &req){
	HttpResponse res;
	std::string path = "./www" + req.getUri();

	std::string queryString = "";
	size_t queryPos = req.getUri().find('?');
	if (queryPos != std::string::npos) {
		queryString = req.getUri().substr(queryPos + 1);
		path = "./www" + req.getUri().substr(0, queryPos);
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

		setenv("REQUEST_METHOD", req.getMethod().c_str(), 1);
		setenv("QUERY_STRING", queryString.c_str(), 1);
		setenv("CONTENT_LENGTH", req.getHeaders().count("Content-Length") ? req.getHeaders().at("Content-Length").c_str() : "", 1);
		setenv("CONTENT_TYPE", req.getHeaders().count("Content-Type") ? req.getHeaders().at("Content-Type").c_str() : "", 1);
		setenv("SCRIPT_NAME", req.getUri().c_str(), 1);
		setenv("SERVER_NAME", req.getHeaders().count("Host") ? req.getHeaders().at("Host").c_str() : "localhost", 1);
		setenv("SERVER_PORT", "8080", 1);
		setenv("SERVER_PROTOCOL", req.getVersion().c_str(), 1);
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

		if (req.getMethod() == "POST" && !req.getBody().empty()) {
			write(inputPipe[1], req.getBody().c_str(), req.getBody().size());
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
    
    // Copiar configuração de error pages
    res.setErrorPageConfig(this->_errorPages, this->_rootPath);
    
    std::string path = this->_rootPath + req.getUri();

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
    {
        res.setErrorPage(404);
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

	res.setErrorPageConfig(this->_errorPages, this->_rootPath);
    
    if (req.getBody().empty())
    {
        res.setErrorPage(400);
        return res;
    }

	std::string uri = req.getUri();
    
    if (uri.find("/upload") != 0)
    {
        res.setErrorPage(404);
        return res;
    }

	// Verificar se há corpo na requisição
	if (req.getBody().empty()) {
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

	file << req.getBody();
	file.close();

	res.setStatus(201, "Created");
	std::ostringstream bodyMsg;
	bodyMsg << "<h1>File uploaded successfully!</h1>"
			<< "<p>File saved to: " << path << "</p>"
			<< "<p>Size: " << req.getBody().size() << " bytes</p>";
	res.setBody(bodyMsg.str(), "text/html");
	return res;
}

HttpResponse	HttpResponse::handleDelete(const HttpRequest &req){
	HttpResponse res;
	res.setErrorPageConfig(this->_errorPages, this->_rootPath);
	
	std::string path = this->_rootPath + req.getUri();

	if (std::remove(path.c_str()) == 0)
	{
		res.setStatus(200, "OK");
		res.setBody("<h1>File deleted successfully</h1>", "text/html");
	}
	else
	{
		res.setErrorPage(404);
	}
	return res;
};

HttpResponse HttpResponse::dispatchRequest(const HttpRequest &req)
{
    std::string extension;
    size_t dotPos = req.getUri().find_last_of('.');
    size_t queryPos = req.getUri().find('?');

    if (dotPos != std::string::npos) {
        if (queryPos != std::string::npos && queryPos > dotPos) {
            extension = req.getUri().substr(dotPos, queryPos - dotPos);
        } else {
            extension = req.getUri().substr(dotPos);
        }
        if (extension == ".py") {
            return handleCGI(req);
        }
    }

    if (req.getMethod() == "GET") {
        return handleGet(req);
    } 
    else if (req.getMethod() == "POST") {
        return handlePost(req);
    } 
    else if (req.getMethod() == "DELETE") {
        return handleDelete(req);
    } 
    else {
        // Método não suportado
        HttpResponse res;
        res.setErrorPageConfig(this->_errorPages, this->_rootPath);
        res.setErrorPage(405);
        return res;
    }
}

void		HttpResponse::setStatus(int code, const std::string &message)
{
    this->status_code = code;
    if (message.empty())
        this->status_message = getDefaultStatusMessage(code);
    else
        this->status_message = message;
}

void		HttpResponse::setHeader(const std::string &key, const std::string &value){
	this->headers[key] = value;
};

void		HttpResponse::setBody(const std::string &b, const std::string &contentType){
	body = b;
	this->headers["Content-Type"] = contentType;
	this->headers["Content-Length"] = intToString(body.size());
}

void HttpResponse::setErrorPage(int code)
{
	std::string path;
	
	// 1. Tentar usar error_page customizada da configuração
	if (_errorPages != NULL && _errorPages->count(code) > 0)
	{
		path = _rootPath + _errorPages->at(code);
		std::cout << "DEBUG: Usando error_page da config: " << path << std::endl;
	}
	else
	{
		// 2. Fallback: usar error_pages/ padrão
		path = "./error_pages/" + intToString(code) + ".html";
		std::cout << "DEBUG: Usando error_page padrão: " << path << std::endl;
	}
	
	// Tentar abrir o arquivo
	std::ifstream file(path.c_str());
	
	if (!file)
	{
		std::cerr << "AVISO: Arquivo de erro não encontrado: " << path << std::endl;
		
		// 3. Fallback final: página HTML genérica
		setStatus(code, getDefaultStatusMessage(code));
		std::string fallbackBody = 
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"  <meta charset=\"UTF-8\">\n"
			"  <title>" + intToString(code) + " " + status_message + "</title>\n"
			"  <style>\n"
			"    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: #f9f9f9; }\n"
			"    h1 { font-size: 6rem; color: #e74c3c; margin: 0; }\n"
			"    h2 { font-size: 2rem; color: #333; }\n"
			"    p { color: #555; }\n"
			"  </style>\n"
			"</head>\n"
			"<body>\n"
			"  <h1>" + intToString(code) + "</h1>\n"
			"  <h2>" + status_message + "</h2>\n"
			"  <p>WebServ/1.0</p>\n"
			"</body>\n"
			"</html>";
		setBody(fallbackBody, "text/html; charset=utf-8");
		return;
	}
	
	// Ler o arquivo de erro
	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	
	setStatus(code, getDefaultStatusMessage(code));
	setBody(buffer.str(), "text/html; charset=utf-8");
}

// Mapeamento padrão de códigos HTTP
std::string HttpResponse::getDefaultStatusMessage(int code)
{
	switch(code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default: return "Error";
	}
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;

	// Status line
	response << http_version << " "
			<< status_code << " "
			<< status_message << "\r\n";

	// Headers obrigatórios
	std::map<std::string, std::string> finalHeaders = headers;
	
	if (finalHeaders.count("Content-Type") == 0)
		finalHeaders["Content-Type"] = "text/html; charset=utf-8";
	
	if (finalHeaders.count("Content-Length") == 0)
		finalHeaders["Content-Length"] = intToString(body.size());
	
	if (finalHeaders.count("Server") == 0)
		finalHeaders["Server"] = "WebServ/1.0";
	
	if (finalHeaders.count("Connection") == 0)
		finalHeaders["Connection"] = "close";

	// Escrever headers
	for (std::map<std::string, std::string>::const_iterator it = finalHeaders.begin();
		it != finalHeaders.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}

	response << "\r\n";
	response << body;

	return response.str();
}

std::string HttpResponse::intToString(int n) const
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}