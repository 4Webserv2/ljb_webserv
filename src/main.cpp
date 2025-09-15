/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/09/14 20:37:14 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
