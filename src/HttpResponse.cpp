/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:36 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/04 10:23:41 by btaveira         ###   ########.fr       */
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

HttpResponse HttpResponse::handleCGI(const HttpRequest &req) {
	HttpResponse res;
	res.setErrorPageConfig(this->_errorPages, this->_rootPath);
	
	std::string path = this->_rootPath + req.getUri();
	std::string queryString = "";
	
	// Extrair query string
	size_t queryPos = req.getUri().find('?');
	if (queryPos != std::string::npos) {
		queryString = req.getUri().substr(queryPos + 1);
		path = this->_rootPath + req.getUri().substr(0, queryPos);
	}
	
	// 1. Verificar se o arquivo existe
	if (access(path.c_str(), F_OK) == -1) {
		std::cerr << "[CGI] Arquivo não encontrado: " << path << std::endl;
		res.setErrorPage(404);
		return res;
	}
	
	// 2. Verificar se o arquivo é executável
	if (access(path.c_str(), X_OK) == -1) {
		std::cerr << "[CGI] Arquivo não é executável: " << path << std::endl;
		res.setErrorPage(403);
		return res;
	}
	
	// 3. Verificar tamanho do corpo (limite de segurança)
	const size_t MAX_CGI_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	if (req.getBody().size() > MAX_CGI_BODY_SIZE) {
		std::cerr << "[CGI] Corpo da requisição muito grande: " 
				<< req.getBody().size() << " bytes" << std::endl;
		res.setErrorPage(413);
		return res;
	}
	
	// 4. Criar pipes
	int inputPipe[2];
	int outputPipe[2];
	
	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
		std::cerr << "[CGI] Erro ao criar pipes: " << strerror(errno) << std::endl;
		res.setErrorPage(500);
		return res;
	}
	
	// 5. Fork do processo
	pid_t pid = fork();
	
	if (pid < 0) {
		// Erro no fork
		close(inputPipe[0]); close(inputPipe[1]);
		close(outputPipe[0]); close(outputPipe[1]);
		std::cerr << "[CGI] Erro no fork: " << strerror(errno) << std::endl;
		res.setErrorPage(500);
		return res;
	}
	
	if (pid == 0) {
		// ========== PROCESSO FILHO ==========
		
		// Redirecionar STDIN
		close(inputPipe[1]);
		if (dup2(inputPipe[0], STDIN_FILENO) == -1) {
			std::cerr << "[CGI Child] Erro no dup2 STDIN" << std::endl;
			exit(1);
		}
		close(inputPipe[0]);
		
		// Redirecionar STDOUT
		close(outputPipe[0]);
		if (dup2(outputPipe[1], STDOUT_FILENO) == -1) {
			std::cerr << "[CGI Child] Erro no dup2 STDOUT" << std::endl;
			exit(1);
		}
		close(outputPipe[1]);
		
		// Redirecionar STDERR para STDOUT (para capturar erros)
		dup2(STDOUT_FILENO, STDERR_FILENO);
		
		// ========== CONFIGURAR VARIÁVEIS DE AMBIENTE CGI/1.1 ==========
		
		// Obrigatórias (RFC 3875)
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", req.getVersion().c_str(), 1);
		setenv("SERVER_SOFTWARE", "WebServ/1.0", 1);
		setenv("REQUEST_METHOD", req.getMethod().c_str(), 1);
		setenv("SCRIPT_NAME", req.getUri().substr(0, queryPos).c_str(), 1);
		setenv("QUERY_STRING", queryString.c_str(), 1);
		
		// Server info
		setenv("SERVER_NAME", req.getHeader("Host").c_str(), 1);
		setenv("SERVER_PORT", "8080", 1);
		
		// Remote info
		setenv("REMOTE_ADDR", "127.0.0.1", 1);
		setenv("REMOTE_HOST", "localhost", 1);
		
		// Content info (se houver body)
		if (!req.getBody().empty()) {
			std::ostringstream contentLength;
			contentLength << req.getBody().size();
			setenv("CONTENT_LENGTH", contentLength.str().c_str(), 1);
			setenv("CONTENT_TYPE", req.getHeader("Content-Type").c_str(), 1);
		} else {
			setenv("CONTENT_LENGTH", "0", 1);
		}
		
		// PATH_INFO e PATH_TRANSLATED (se aplicável)
		setenv("PATH_INFO", "", 1);
		setenv("PATH_TRANSLATED", "", 1);
		
		// Outros headers como HTTP_*
		std::map<std::string, std::string> headers = req.getHeaders();
		for (std::map<std::string, std::string>::iterator it = headers.begin();
			it != headers.end(); ++it) {
			std::string envName = "HTTP_" + it->first;
			// Converter para maiúsculas e trocar - por _
			for (size_t i = 0; i < envName.length(); i++) {
				envName[i] = std::toupper(envName[i]);
				if (envName[i] == '-')
					envName[i] = '_';
			}
			setenv(envName.c_str(), it->second.c_str(), 1);
		}
		
		// REDIRECT_STATUS (necessário para PHP-CGI)
		setenv("REDIRECT_STATUS", "200", 1);
		
		// ========== MUDAR DIRETÓRIO DE TRABALHO ==========
		std::string scriptDir = path.substr(0, path.find_last_of('/'));
		if (chdir(scriptDir.c_str()) == -1) {
			std::cerr << "[CGI Child] Erro ao mudar diretório: " 
					<< strerror(errno) << std::endl;
			exit(1);
		}
		
		// ========== EXECUTAR O SCRIPT ==========
		std::string scriptName = path.substr(path.find_last_of('/') + 1);
		
		char* const args[] = {
			const_cast<char*>("python3"),
			const_cast<char*>(scriptName.c_str()),
			NULL
		};
		
		execve("/usr/bin/python3", args, environ);
		
		// Se execve falhar
		std::cerr << "[CGI Child] Erro no execve: " << strerror(errno) << std::endl;
		exit(1);
	}
	
	// ========== PROCESSO PAI ==========
	
	close(inputPipe[0]);
	close(outputPipe[1]);
	
	// 6. Enviar corpo da requisição ao CGI (se houver)
	if (req.getMethod() == "POST" && !req.getBody().empty()) {
		ssize_t written = write(inputPipe[1], req.getBody().c_str(), req.getBody().size());
		if (written < 0) {
			std::cerr << "[CGI] Erro ao escrever no pipe: " << strerror(errno) << std::endl;
		}
	}
	close(inputPipe[1]);
	
	// 7. Ler saída do CGI com timeout
	std::string cgiOutput;
	char buffer[4096];
	ssize_t bytesRead;
	
	// Configurar timeout de 30 segundos
	fd_set readFds;
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	
	while (true) {
		FD_ZERO(&readFds);
		FD_SET(outputPipe[0], &readFds);
		
		int selectResult = select(outputPipe[0] + 1, &readFds, NULL, NULL, &timeout);
		
		if (selectResult == -1) {
			std::cerr << "[CGI] Erro no select: " << strerror(errno) << std::endl;
			break;
		} else if (selectResult == 0) {
			// Timeout
			std::cerr << "[CGI] Timeout ao ler saída do CGI" << std::endl;
			kill(pid, SIGKILL);
			break;
		}
		
		bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1);
		if (bytesRead <= 0)
			break;
		
		buffer[bytesRead] = '\0';
		cgiOutput += buffer;
		
		// Limite de segurança: 10MB de output
		if (cgiOutput.size() > 10 * 1024 * 1024) {
			std::cerr << "[CGI] Output muito grande, abortando" << std::endl;
			kill(pid, SIGKILL);
			break;
		}
	}
	
	close(outputPipe[0]);
	
	// 8. Aguardar término do processo filho
	int status;
	int waitResult = waitpid(pid, &status, 0);
	
	if (waitResult == -1) {
		std::cerr << "[CGI] Erro no waitpid: " << strerror(errno) << std::endl;
		res.setErrorPage(500);
		return res;
	}
	
	// 9. Verificar status de saída
	if (WIFEXITED(status)) {
		int exitCode = WEXITSTATUS(status);
		std::cout << "[CGI] Processo terminou com código: " << exitCode << std::endl;
		
		if (exitCode != 0) {
			std::cerr << "[CGI] Script retornou código de erro: " << exitCode << std::endl;
			res.setErrorPage(502);
			return res;
		}
	} else if (WIFSIGNALED(status)) {
		int signal = WTERMSIG(status);
		std::cerr << "[CGI] Processo morreu com sinal: " << signal << std::endl;
		res.setErrorPage(502);
		return res;
	}
	
	// 10. Parsear saída do CGI
	if (cgiOutput.empty()) {
		std::cerr << "[CGI] Output vazio" << std::endl;
		res.setErrorPage(502);
		return res;
	}
	
	// Separar headers e body
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		headerEnd = cgiOutput.find("\n\n");
	}
	
	if (headerEnd != std::string::npos) {
		std::string headers = cgiOutput.substr(0, headerEnd);
		std::string body = cgiOutput.substr(headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2));
		
		// Parsear headers do CGI
		std::istringstream headerStream(headers);
		std::string line;
		bool hasStatus = false;
		
		while (std::getline(headerStream, line)) {
			if (line.empty() || line == "\r")
				continue;
			
			// Remover \r
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			
			size_t colonPos = line.find(':');
			if (colonPos != std::string::npos) {
				std::string key = line.substr(0, colonPos);
				std::string value = line.substr(colonPos + 1);
				
				// Trim do valor
				while (!value.empty() && std::isspace(value[0]))
					value.erase(0, 1);
				while (!value.empty() && std::isspace(value[value.length() - 1]))
					value.erase(value.length() - 1);
				
				if (key == "Status") {
					hasStatus = true;
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
		
		// Se não tem status, assumir 200 OK
		if (!hasStatus)
			res.setStatus(200, "OK");
		
		// Configurar body
		if (res.headers.count("Content-Type") == 0) {
			res.setBody(body, "text/html; charset=utf-8");
		} else {
			res.body = body;
			res.headers["Content-Length"] = res.intToString(body.size());
		}
	} else {
		// Sem headers, assumir que tudo é body HTML
		res.setStatus(200, "OK");
		res.setBody(cgiOutput, "text/html; charset=utf-8");
	}
	
	std::cout << "[CGI] Requisição processada com sucesso" << std::endl;
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