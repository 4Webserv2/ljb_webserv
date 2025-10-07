/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/06 19:16:38 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"

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

void testHttpParserRobusto() {
	std::cout << "\n==== Teste Robusto do HttpParser ====\n" << std::endl;
	HttpParser parser;

	// 1. Request válido
	try {
		std::string req1 =
			"GET /ok HTTP/1.1\r\n"
			"Host: test\r\n"
			"\r\n"
			"body";
		HttpRequest r = parser.httpParser(req1);
		std::cout << "[OK] Request válido passou." << std::endl;
	} catch (std::exception &e) {
		std::cout << "[ERRO] Request válido falhou: " << e.what() << std::endl;
	}

	// 2. Método não permitido
	try {
		std::string req2 =
			"PUT /fail HTTP/1.1\r\n"
			"Host: test\r\n"
			"\r\n";
		parser.httpParser(req2);
		std::cout << "[ERRO] Método não permitido NÃO lançou exceção!" << std::endl;
	} catch (std::exception &e) {
		std::cout << "[OK] Método não permitido lançou exceção: " << e.what() << std::endl;
	}

	// 3. Request line malformada (faltando versão)
	try {
		std::string req3 =
			"GET /semversao\r\n"
			"Host: test\r\n"
			"\r\n";
		parser.httpParser(req3);
		std::cout << "[ERRO] Request line malformada NÃO lançou exceção!" << std::endl;
	} catch (std::exception &e) {
		std::cout << "[OK] Request line malformada lançou exceção: " << e.what() << std::endl;
	}

	// 4. Header malformado (sem dois pontos)
	try {
		std::string req4 =
			"GET / HTTP/1.1\r\n"
			"Host test\r\n"
			"\r\n";
		HttpRequest r = parser.httpParser(req4);
		if (r.headers.find("Host") == r.headers.end())
			std::cout << "[OK] Header malformado ignorado." << std::endl;
		else
			std::cout << "[ERRO] Header malformado foi aceito!" << std::endl;
	} catch (std::exception &e) {
		std::cout << "[ERRO] Header malformado lançou exceção: " << e.what() << std::endl;
	}

	// 5. Request vazio
	try {
		std::string req5 = "";
		parser.httpParser(req5);
		std::cout << "[ERRO] Request vazio NÃO lançou exceção!" << std::endl;
	} catch (std::exception &e) {
		std::cout << "[OK] Request vazio lançou exceção: " << e.what() << std::endl;
	}

	// 6. Request line só com método
	try {
		std::string req6 = "GET\r\n\r\n";
		parser.httpParser(req6);
		std::cout << "[ERRO] Request line incompleta NÃO lançou exceção!" << std::endl;
	} catch (std::exception &e) {
		std::cout << "[OK] Request line incompleta lançou exceção: " << e.what() << std::endl;
	}

	// 7. Request com múltiplos headers e body
	try {
		std::string req7 =
			"POST /multi HTTP/1.1\r\n"
			"Host: test\r\n"
			"X-Test: 123\r\n"
			"\r\n"
			"linha1\nlinha2";
		HttpRequest r = parser.httpParser(req7);
		if (r.headers["Host"] == "test" && r.headers["X-Test"] == "123" && r.body.find("linha2") != std::string::npos)
			std::cout << "[OK] Request com múltiplos headers e body passou." << std::endl;
		else
			std::cout << "[ERRO] Falha ao processar múltiplos headers/body." << std::endl;
	} catch (std::exception &e) {
		std::cout << "[ERRO] Request com múltiplos headers/body lançou exceção: " << e.what() << std::endl;
	}
	std::cout << "==== Fim dos testes robustos ====\n" << std::endl;
}

int	main() {
	testHttpParserRobusto();
	////////////////////////////////////////////////////////////////

	test();
	/*int serverFd = socket(AF_INET, SOCK_STREAM, 0);
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
	serverLoop(serverFd);*/
	return (0);
}
