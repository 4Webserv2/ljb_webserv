/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/11/29 15:13:02 by jbergfel         ###   ########.fr       */
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

		request.setPar(request.httpParse(rawRequest));
		HttpResponse response;
		response = response.dispatchRequest(request);
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
				handler->EpollEventHandler(data);
		}
	}
}

void	serverLoop()
{
	while (42)
	{
		int sockets = EpollInstance::manipEpollWait();
		if (sockets == -1)
		{
			std::cout << "Erro ao aceitar conexão do cliente" << std::endl;
			break;
		}
		else
		{
			epollReadyListLoop(sockets);
			epollValidationLoop();
		}
	}
}

int	main(int ac, char **av)
{
	if (RunTime::createRuntime(ac, av) == 0)
		serverLoop();
	else
	{
		std::cout << "Error" << std::endl;
		return (1);
	}
	return (0);
}
