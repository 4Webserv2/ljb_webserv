/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/09/23 13:06:26 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include <cstdlib>
#include <cstring>

int	clientLoop(const int& clientFd) {
	char	buffer[BUFFER_SIZE];

	while (42) {
		int	bytesRead = recv(clientFd, buffer, BUFFER_SIZE -1 , 0);
		if (bytesRead == -1) {
			std::cout << "Erro na leitura do cliente" << std::endl;
			close(clientFd);
			return (ERROR);
		}
		if (bytesRead == 0) {
			std::cout << "Cliente desconectou" << std::endl;
			close(clientFd);
			return (NO_ERROR);
		}
		buffer[bytesRead] = '\0';
		std::cout << "Recebido (" << bytesRead << " bytes): " << buffer << std::endl;

		send(clientFd, buffer, bytesRead, 0);
	}
	return (0);
}

int	serverLoop(const int& serverFd) {
	while (42) {
		struct sockaddr_in clientAddr;
		socklen_t	clientAddrLen = sizeof(clientAddr);

		int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
		if (clientFd == -1) {
			std::cout << "Erro ao aceitar conexão do cliente" << std::endl;
			//Não precisa parar/ sair do loop!
		}
		std::cout << "Cliente conectado: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

		send(clientFd, "Olá do lado do servidor!\n", 27, 0);
		clientLoop(clientFd);
	}
	return (0);
}

int	main() {
	// Teste do HttpParser
    std::string rawRequest =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: TestClient/1.0\r\n"
        "\r\n"
        "corpo do request";

    HttpParser parser;
    HttpRequest req = parser.httpParser(rawRequest);

    std::cout << "Método: " << req.method << std::endl;
    std::cout << "URI: " << req.uri << std::endl;
    std::cout << "Versão: " << req.version << std::endl;
    for (std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
        std::cout << "Header: " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "Body: " << req.body << std::endl;
	std::string postRequest =
        "POST /api/data HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 17\r\n"
        "\r\n"
        "{\"key\":\"value\"}";

    HttpRequest postReq = parser.httpParser(postRequest);
    std::cout << "Método: " << postReq.method << std::endl;
    std::cout << "URI: " << postReq.uri << std::endl;
    std::cout << "Versão: " << postReq.version << std::endl;
    for (std::map<std::string, std::string>::iterator it = postReq.headers.begin(); it != postReq.headers.end(); ++it) {
        std::cout << "Header: " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "Body: " << postReq.body << std::endl;

    // Teste com DELETE
    std::string deleteRequest =
        "DELETE /api/item/1 HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";

    HttpRequest deleteReq = parser.httpParser(deleteRequest);
    std::cout << "Método: " << deleteReq.method << std::endl;
    std::cout << "URI: " << deleteReq.uri << std::endl;
    std::cout << "Versão: " << deleteReq.version << std::endl;
    for (std::map<std::string, std::string>::iterator it = deleteReq.headers.begin(); it != deleteReq.headers.end(); ++it) {
        std::cout << "Header: " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "Body: " << deleteReq.body << std::endl;
	////////////////////////////////////////////////////////////////



	
	int serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1) {
		std::cout << "Erro ao criar socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "Socket criado" << std::endl;
	struct sockaddr_in serverAddr;
	socklen_t	serverAddrLen = sizeof(serverAddr);

	bzero(&serverAddr, serverAddrLen);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFd, (const struct sockaddr *)&serverAddr, serverAddrLen) == -1) {
		std::cout << "Erro ao bindar socket" << std::endl;
		close(serverFd);
		exit(EXIT_FAILURE);
	}
	std::cout << "Socket bindado" << std::endl;
	if (listen(serverFd, BACKLOG) == -1) {
		std::cout << "Erro ao colocar socket em modo passivo" << std::endl;
		close(serverFd);
		exit(EXIT_FAILURE);
	}
	std::cout << "Servidor ouvindo na porta " << PORT << std::endl;
	serverLoop(serverFd);
	return (0);
}
