/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:36 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/23 19:54:53 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpResponse.hpp"
# include "../includes/StringUtils.hpp"
# include "../includes/Logger.hpp"

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
		StringUtils::errorAndCerr("[CGI] File not found: " + path);

		res.setErrorPage(404);
		return res;
	}

	// 2. Verificar se o arquivo é executável
	if (access(path.c_str(), X_OK) == -1) {
		StringUtils::errorAndCerr("[CGI] File is not executable: " + path);

		res.setErrorPage(403);
		return res;
	}

	// 3. Verificar tamanho do corpo (limite de segurança)
	const size_t MAX_CGI_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	if (req.getBody().size() > MAX_CGI_BODY_SIZE) {
		StringUtils::errorAndCerr("[CGI] Request body is too large: " +
			StringUtils::intToString(req.getBody().size()) + " bytes");

		res.setErrorPage(413);
		return res;
	}

	// 4. Criar pipes
	int inputPipe[2];
	int outputPipe[2];

	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
		StringUtils::errorAndCerr(std::string("[CGI] Error creating pipes: ") + strerror(errno));

		res.setErrorPage(500);
		return res;
	}

	// 5. Fork do processo
	pid_t pid = fork();

	if (pid < 0) {
		// Erro no fork
		close(inputPipe[0]); close(inputPipe[1]);
		close(outputPipe[0]); close(outputPipe[1]);

		StringUtils::errorAndCerr(std::string("[CGI] Error on fork: ") + strerror(errno));
		res.setErrorPage(500);
		return res;
	}

	if (pid == 0) {
		// ========== PROCESSO FILHO ==========

		// Redirecionar STDIN
		close(inputPipe[1]);
		if (dup2(inputPipe[0], STDIN_FILENO) == -1) {
			StringUtils::errorAndCerr("[CGI Child] Error on dup2 STDIN");
			exit(1);
		}
		close(inputPipe[0]);

		// Redirecionar STDOUT
		close(outputPipe[0]);
		if (dup2(outputPipe[1], STDOUT_FILENO) == -1) {
			StringUtils::errorAndCerr("[CGI Child] Error on dup2 STDOUT");
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
			StringUtils::errorAndCerr(std::string("[CGI Child] Error changing directory: ") + strerror(errno));
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

		StringUtils::errorAndCerr(std::string("[CGI Child] Error on execve: ") + strerror(errno));
		exit(1);
	}

	// ========== PROCESSO PAI ==========

	close(inputPipe[0]);
	close(outputPipe[1]);

	// 6. Enviar corpo da requisição ao CGI (se houver)
	if (req.getMethod() == "POST" && !req.getBody().empty()) {
		ssize_t written = write(inputPipe[1], req.getBody().c_str(), req.getBody().size());
		if (written < 0) {
			StringUtils::errorAndCerr(std::string("[CGI] Error writing to pipe: ") + strerror(errno));
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
			StringUtils::errorAndCerr(std::string("[CGI] Error on select: ") + strerror(errno));
			break;
		} else if (selectResult == 0) {
			// Timeout
			StringUtils::errorAndCerr("[CGI] Timeout while reading CGI output");
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
			StringUtils::errorAndCerr("[CGI] Output too large, aborting");
			kill(pid, SIGKILL);
			break;
		}
	}

	close(outputPipe[0]);

	// 8. Aguardar término do processo filho
	int status;
	int waitResult = waitpid(pid, &status, 0);

	if (waitResult == -1) {
		StringUtils::errorAndCerr(std::string("[CGI] Error on waitpid: ") + strerror(errno));
		res.setErrorPage(500);
		return res;
	}

	// 9. Verificar status de saída
	if (WIFEXITED(status)) {
		int exitCode = WEXITSTATUS(status);

		StringUtils::errorAndCerr("[CGI] Process exited with code: "
			+ StringUtils::intToString(exitCode));

		if (exitCode != 0) {
			StringUtils::errorAndCerr("[CGI] Script returned a non-zero exit code: "
				+ StringUtils::intToString(exitCode));
			res.setErrorPage(502);
			return res;
		}
	}
	else if (WIFSIGNALED(status)) {
		int signal = WTERMSIG(status);

		StringUtils::errorAndCerr("[CGI] Process terminated by signal: "
			+ StringUtils::intToString(signal));
		res.setErrorPage(502);
		return res;
	}

	// 10. Parsear saída do CGI
	if (cgiOutput.empty()) {
		StringUtils::errorAndCerr("[CGI] Empty CGI output");
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

	Logger::info("[CGI] Request processed successfully");
	return res;
}


HttpResponse	HttpResponse::handleGet(const HttpRequest &req) {
    HttpResponse res;
    res.setErrorPageConfig(this->_errorPages, this->_rootPath);

    std::string path = this->_rootPath + req.getUri();

    // 1. Verificar se o path é um diretório
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode)) {
        std::cout << "[GET] Path é um diretório: " << path << std::endl;

        // Lista de arquivos index para tentar (em ordem de prioridade)
        std::vector<std::string> indexFiles;
        indexFiles.push_back("index.html");
        indexFiles.push_back("index.htm");

        // Tentar cada arquivo index
        for (size_t i = 0; i < indexFiles.size(); i++) {
            std::string indexPath = path;

            // Adicionar '/' se necessário
            if (indexPath[indexPath.length() - 1] != '/')
                indexPath += "/";

            indexPath += indexFiles[i];

            std::cout << "[GET] Tentando index: " << indexPath << std::endl;

            // Verificar se o arquivo existe
            if (access(indexPath.c_str(), F_OK) == 0) {
                std::cout << "[GET] ✅ Index encontrado: " << indexPath << std::endl;
                path = indexPath;
                break;
            }
        }
    }

    // 2. Tentar abrir o arquivo
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << "[GET] ❌ Arquivo não encontrado: " << path << std::endl;
        res.setErrorPage(404);
        return res;
    }

    // 3. Ler conteúdo do arquivo
    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // 4. Determinar Content-Type baseado na extensão
    std::string contentType = "text/html; charset=utf-8";

    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);

        if (ext == ".css")
            contentType = "text/css";
        else if (ext == ".js")
            contentType = "application/javascript";
        else if (ext == ".json")
            contentType = "application/json";
        else if (ext == ".png")
            contentType = "image/png";
        else if (ext == ".jpg" || ext == ".jpeg")
            contentType = "image/jpeg";
        else if (ext == ".gif")
            contentType = "image/gif";
        else if (ext == ".svg")
            contentType = "image/svg+xml";
        else if (ext == ".pdf")
            contentType = "application/pdf";
        else if (ext == ".txt")
            contentType = "text/plain";
    }

    std::cout << "[GET] ✅ Servindo arquivo: " << path
              << " (" << buffer.str().size() << " bytes, "
              << contentType << ")" << std::endl;

    res.setStatus(200, "OK");
    res.setBody(buffer.str(), contentType);
    return res;
};

HttpResponse	HttpResponse::handlePost(const HttpRequest &req, const LocationBlock &location){
    HttpResponse res;
    res.setErrorPageConfig(this->_errorPages, this->_rootPath);

    std::string uri = req.getUri();

    // 1. Verificar se a URI suporta POST
    bool isUploadEndpoint = (uri.find("/upload") == 0);
    bool isPythonScript = (uri.find(".py") != std::string::npos);
    bool isBlaScript = (uri.find(".bla") != std::string::npos);

    if (!isUploadEndpoint && !isPythonScript && !isBlaScript)
    {
        std::cerr << "[POST] URI não suporta POST: " << uri << std::endl;
        res.setErrorPage(405);
        return res;
    }

    // 2. Se for script (.py ou .bla), redirecionar para CGI
    if (isPythonScript || isBlaScript)
    {
        std::cout << "[POST] Redirecionando para handleCGI" << std::endl;
        return handleCGI(req);
    }

    // 3. Processar upload (apenas para /upload)

    // Se o body estiver vazio, retornar 200 OK
    if (req.getBody().empty()) {
        std::cout << "[POST] Body vazio, retornando 200 OK" << std::endl;
        res.setStatus(200, "OK");
        res.setBody(
            "<h1>200 OK</h1>"
            "<p>POST request processed successfully.</p>"
            "<p>No data was uploaded (empty body).</p>",
            "text/html"
        );
        return res;
    }

    // Criar diretório uploads se não existir
	std::cout << location.getUploadPath();
    std::string uploadDir = "uploads";//location.getUploadPath();
    struct stat st;
    if (stat(uploadDir.c_str(), &st) == -1) {
        if (mkdir(uploadDir.c_str(), 0755) != 0) {
            std::cerr << "[POST] Erro ao criar diretório uploads: "
                      << strerror(errno) << std::endl;
            res.setErrorPage(500);
            return res;
        }
    }

    // Verificar se é multipart/form-data (upload de arquivo)
    std::string contentType = req.getHeader("Content-Type");

    if (contentType.find("multipart/form-data") != std::string::npos) {
        // Upload de arquivo com multipart
        std::cout << "[POST] Processando multipart/form-data upload" << std::endl;
        return handleMultipartUpload(req, uploadDir, res);
    } else {
        // Upload simples (texto/dados brutos)
        std::cout << "[POST] Processando upload simples" << std::endl;
        return handleSimpleUpload(req, uploadDir, res);
    }
}

// Novo método: Upload simples
HttpResponse HttpResponse::handleSimpleUpload(const HttpRequest &req,
                                               const std::string &uploadDir,
                                               HttpResponse &res)
{
    // Gerar nome de arquivo único (timestamp)
    std::time_t now = std::time(0);
    std::ostringstream filename;
    filename << uploadDir << "/upload_" << now << ".txt";

    std::string path = filename.str();

    // Salvar arquivo
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << "[POST] Erro ao criar arquivo: " << path << std::endl;
        res.setErrorPage(500);
        return res;
    }

    file << req.getBody();
    file.close();

    std::cout << "[POST] Upload concluído: " << path
              << " (" << req.getBody().size() << " bytes)" << std::endl;

    res.setStatus(201, "Created");
    std::ostringstream bodyMsg;
    bodyMsg << "<h1>✅ File uploaded successfully!</h1>"
            << "<p><strong>File saved to:</strong> " << path << "</p>"
            << "<p><strong>Size:</strong> " << req.getBody().size() << " bytes</p>";
    res.setBody(bodyMsg.str(), "text/html");
    return res;
}

// Novo método: Upload multipart (arquivos)
HttpResponse HttpResponse::handleMultipartUpload(const HttpRequest &req,
                                                  const std::string &uploadDir,
                                                  HttpResponse &res)
{
    std::string contentType = req.getHeader("Content-Type");
    std::string body = req.getBody();

    std::cout << "[Multipart] Content-Type: " << contentType << std::endl;
    std::cout << "[Multipart] Body size: " << body.size() << " bytes" << std::endl;

    // Extrair boundary do Content-Type
    // Ex: multipart/form-data; boundary=----WebKitFormBoundaryFR0Yj9bGrUMTGTjp
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        std::cerr << "[POST] Boundary não encontrado no Content-Type" << std::endl;
        res.setErrorPage(400);
        return res;
    }

    // MELHORAR: Remover possíveis aspas ao redor do boundary
    std::string boundaryValue = contentType.substr(boundaryPos + 9);

    // Remover espaços, tabs, aspas
    while (!boundaryValue.empty() &&
           (boundaryValue[0] == ' ' || boundaryValue[0] == '\t' || boundaryValue[0] == '"')) {
        boundaryValue.erase(0, 1);
    }
    while (!boundaryValue.empty() &&
           (boundaryValue[boundaryValue.length()-1] == ' ' ||
            boundaryValue[boundaryValue.length()-1] == '\t' ||
            boundaryValue[boundaryValue.length()-1] == '"' ||
            boundaryValue[boundaryValue.length()-1] == '\r' ||
            boundaryValue[boundaryValue.length()-1] == '\n')) {
        boundaryValue.erase(boundaryValue.length()-1);
    }

    std::string boundary = "--" + boundaryValue;
    std::cout << "[Multipart] Boundary: '" << boundary << "'" << std::endl;

    // Debug: Mostrar primeiros 200 chars do body
    std::cout << "[Multipart] Body preview: "
              << body.substr(0, std::min((size_t)200, body.size())) << std::endl;

    // Procurar pelo primeiro boundary
    size_t startPos = body.find(boundary);
    if (startPos == std::string::npos) {
        std::cerr << "[POST] Boundary '" << boundary << "' não encontrado no body" << std::endl;
        res.setErrorPage(400);
        return res;
    }

    std::cout << "[Multipart] Primeiro boundary encontrado na posição: " << startPos << std::endl;

    std::vector<std::string> uploadedFiles;
    int partCount = 0;

    // Processar cada parte do multipart
    while (startPos != std::string::npos) {
        partCount++;
        std::cout << "[Multipart] Processando parte #" << partCount << std::endl;

        // Encontrar o próximo boundary
        size_t nextBoundary = body.find(boundary, startPos + boundary.length());
        if (nextBoundary == std::string::npos)
            break;

        // Extrair a parte entre os boundaries
        std::string part = body.substr(startPos + boundary.length(),
                                       nextBoundary - startPos - boundary.length());

        std::cout << "[Multipart] Tamanho da parte: " << part.size() << " bytes" << std::endl;

        // Procurar pelos headers da parte (terminam com \r\n\r\n)
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            headerEnd = part.find("\n\n");

        if (headerEnd != std::string::npos) {
            std::string headers = part.substr(0, headerEnd);
            std::string fileData = part.substr(headerEnd +
                                              (part[headerEnd] == '\r' ? 4 : 2));

            std::cout << "[Multipart] Headers da parte: " << headers << std::endl;

            // Remover \r\n no final do fileData
            while (!fileData.empty() &&
                   (fileData[fileData.length() - 1] == '\r' ||
                    fileData[fileData.length() - 1] == '\n')) {
                fileData.erase(fileData.length() - 1);
            }

            std::cout << "[Multipart] Dados do arquivo: " << fileData.size() << " bytes" << std::endl;

            // Extrair filename do header Content-Disposition
            std::string filename = extractFilename(headers);
            std::cout << "[Multipart] Filename extraído: '" << filename << "'" << std::endl;

            if (!filename.empty() && !fileData.empty()) {
                // Gerar path único
                std::time_t now = std::time(0);
                std::ostringstream filepath;
                filepath << uploadDir << "/" << now << "_" << filename;

                // Salvar arquivo
                std::ofstream file(filepath.str().c_str(), std::ios::binary);
                if (file) {
                    file.write(fileData.c_str(), fileData.size());
                    file.close();

                    uploadedFiles.push_back(filepath.str());
                    std::cout << "[POST] ✅ Arquivo salvo: " << filepath.str()
                              << " (" << fileData.size() << " bytes)" << std::endl;
                } else {
                    std::cerr << "[POST] ❌ Erro ao salvar arquivo: "
                              << filepath.str() << std::endl;
                }
            } else {
                std::cout << "[Multipart] ⚠️ Parte ignorada (filename vazio ou sem dados)" << std::endl;
            }
        }

        startPos = nextBoundary;
    }

    std::cout << "[Multipart] Total de partes processadas: " << partCount << std::endl;
    std::cout << "[Multipart] Arquivos salvos: " << uploadedFiles.size() << std::endl;

    // Montar resposta
    if (uploadedFiles.empty()) {
        res.setStatus(400, "Bad Request");
        res.setBody("<h1>❌ No files uploaded</h1>"
                   "<p>No valid files were found in the request.</p>", "text/html");
        return res;
    }

    res.setStatus(201, "Created");
    std::ostringstream bodyMsg;
    bodyMsg << "<h1>✅ Files uploaded successfully!</h1>";
    bodyMsg << "<p><strong>Total files:</strong> " << uploadedFiles.size() << "</p>";
    bodyMsg << "<h2>Uploaded files:</h2><ul>";

    for (size_t i = 0; i < uploadedFiles.size(); i++) {
        bodyMsg << "<li>" << uploadedFiles[i] << "</li>";
    }
    bodyMsg << "</ul>";

    res.setBody(bodyMsg.str(), "text/html");
    return res;
}

// Função auxiliar: extrair filename do header Content-Disposition
std::string HttpResponse::extractFilename(const std::string &headers)
{
    // Procurar por filename="..."
    size_t filenamePos = headers.find("filename=\"");
    if (filenamePos == std::string::npos)
        return "";

    size_t start = filenamePos + 10; // Pular filename="
    size_t end = headers.find("\"", start);

    if (end == std::string::npos)
        return "";

    std::string filename = headers.substr(start, end - start);

    // Sanitizar filename (remover path traversal)
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        filename = filename.substr(lastSlash + 1);

    return filename;
}

HttpResponse	HttpResponse::handleDelete(const HttpRequest &req){
    HttpResponse res;
    res.setErrorPageConfig(this->_errorPages, this->_rootPath);

    std::string uri = req.getUri();
    std::string path;

    std::cout << "[DELETE] URI: " << uri << std::endl;

    // ADICIONAR: Verificar se é um arquivo em /uploads (fora de www/)
    if (uri.find("/uploads/") == 0) {
        // Arquivo está em ./uploads/ (raiz do projeto)
        path = "." + uri; // ./uploads/arquivo.pdf
        std::cout << "[DELETE] Caminho de upload: " << path << std::endl;
    } else {
        // Arquivo está em ./www/
        path = this->_rootPath + uri;
        std::cout << "[DELETE] Caminho em www: " << path << std::endl;
    }

    // Verificar se o arquivo existe
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        std::cerr << "[DELETE] ❌ Arquivo não encontrado: " << path << std::endl;
        std::cerr << "[DELETE] Erro: " << strerror(errno) << std::endl;
        res.setErrorPage(404);
        return res;
    }

    // Verificar se é um arquivo regular (não diretório)
    if (!S_ISREG(st.st_mode)) {
        std::cerr << "[DELETE] ❌ Não é um arquivo regular: " << path << std::endl;
        res.setErrorPage(403);
        return res;
    }

    // Tentar deletar
    if (std::remove(path.c_str()) == 0)
    {
        std::cout << "[DELETE] ✅ Arquivo deletado com sucesso: " << path << std::endl;
        res.setStatus(200, "OK");
        res.setBody("<h1>✅ File deleted successfully</h1><p>File: " + uri + "</p>", "text/html");
    }
    else
    {
        std::cerr << "[DELETE] ❌ Erro ao deletar arquivo: " << path << std::endl;
        std::cerr << "[DELETE] Erro: " << strerror(errno) << std::endl;
        res.setErrorPage(500);
    }
    return res;
}

HttpResponse HttpResponse::dispatchRequest(const HttpRequest &req, const LocationBlock &location)
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
		return handlePost(req, location);
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
		Logger::debug("Using configured error_page: " + path);
	}
	else
	{
		// 2. Fallback: usar error_pages/ padrão
		path = "./error_pages/" + intToString(code) + ".html";
		Logger::debug("Using default error_page: " + path);
	}

	// Tentar abrir o arquivo
	std::ifstream file(path.c_str());

	if (!file)
	{
		Logger::warning("Error file not found: " + path);
		std::cerr << "Warning: Error file not found: " << path << std::endl;

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
