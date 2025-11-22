/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/11/21 22:24:42 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"
#include "includes/Runtime.hpp"
#include "includes/HttpRequest.hpp"
#include "includes/HttpResponse.hpp"
#include "includes/ServerManage.hpp"

int	clientLoop(const int& clientFd) {
	char	buffer[BUFFER_SIZE];
	HttpRequest request;
	std::string rawRequest;
	int bytesRead;

	do
	{
		bytesRead = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
		if (bytesRead <= 0)
		{
			if (bytesRead == 0)
				std::cout << "Cliente desconectou" << std::endl;
			else
				std::cout << "Erro na leitura do cliente" << std::endl;
			close(clientFd);
			return (bytesRead == 0 ? NO_ERROR : ERROR);
		}
		buffer[bytesRead] = '\0';
		rawRequest += buffer;
		if (rawRequest.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	} while (bytesRead == BUFFER_SIZE - 1);
	std::cout << "Requisição recebida, processando..." << std::endl;
	try
	{
		HttpParse req = request.httpParse(rawRequest);
		HttpResponse response;
		response = response.dispatchRequest(req);
		std::string responseStr = response.toString();
		send(clientFd, responseStr.c_str(), responseStr.length(), 0);
		std::cout << "Resposta enviada com sucesso (status " << response.intToString(response.status_code) << ")" << std::endl;
	}
	catch (std::exception& e)
	{
		HttpResponse errorResponse;
		errorResponse.setErrorPage(400);
		std::string responseStr = errorResponse.toString();
		send(clientFd, responseStr.c_str(), responseStr.length(), 0);
		std::cout << "Erro ao processar requisição: " << e.what() << std::endl;
	}
	close(clientFd);
	return NO_ERROR;
}

void epollValidationLoop()
{
	std::map<int, EpollHandler *> &handlers = EpollInstance::getEpollHandlers();
	for (std::map<int, EpollHandler *>::iterator it = handlers.begin(); it != handlers.end(); ++it)
	{
		std::cout << "Checking timeout for FD: " << it->first << std::endl;
		it->second->handleTimeout();
	}
}

void epollReadyListLoop(int numberOfReadySockets)
{
	if (numberOfReadySockets)
	{
		for (int i = 0; i < numberOfReadySockets; i++)
		{
			struct epoll_event &data = EpollInstance::getElementFromEventsList(i);

			EpollHandler *handler = static_cast<EpollHandler *>(data.data.ptr);
			if (handler)
			{
				handler->EpollEventHandler(data);
			}
		}
	}
}

int	serverLoop()
{
	while (42)
	{
		int sockets = EpollInstance::manipEpollWait();
		if (sockets == -1)
		{
			std::cout << "Erro ao aceitar conexão do cliente" << std::endl;
			break; // Continuar esperando novas conexões
		}
		else
		{

		}
	}
	return 0;
}

int	main(int ac, char **av)
{
	(void) ac;
	(void) av;

	RunTime::createRuntime(ac, av);
	/*int serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1)
	{
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
	std::cout << "Servidor ouvindo na porta " << PORT << std::endl;*/

	serverLoop();
	return (0);
}
