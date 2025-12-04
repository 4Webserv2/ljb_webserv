/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/04 10:11:13 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"
#include "includes/Runtime.hpp"
#include "includes/HttpRequest.hpp"
#include "includes/HttpResponse.hpp"
#include "includes/ServerManage.hpp"
#include "includes/SignalHandler.hpp"

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
	// Verificar se deve fazer shutdown
	if (SignalHandler::isShutdownRequested()) {
		std::cout << "[MAIN] Shutdown solicitado, parando loop..." << std::endl;
		return;
	}
	
	std::map<int, EpollHandler *> &handlers = EpollInstance::getEpollHandlers();
	for (std::map<int, EpollHandler *>::iterator it = handlers.begin(); 
		 it != handlers.end(); ++it)
	{
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

void serverLoop()
{
	std::cout << "[MAIN] Servidor iniciado. Pressione Ctrl+C para parar." << std::endl;
	
	while (RunTime::isRunning() && !SignalHandler::isShutdownRequested())
	{
		int sockets = EpollInstance::manipEpollWait();
		
		// Se epoll_wait foi interrompido por sinal
		if (sockets == -1)
		{
			if (errno == EINTR) {
				// Interrompido por sinal, verificar se é shutdown
				if (SignalHandler::isShutdownRequested()) {
					std::cout << "[MAIN] epoll_wait interrompido por sinal de shutdown" 
							  << std::endl;
					break;
				}
				// Outro sinal, continuar
				continue;
			}
			
			std::cerr << "[ERROR] Erro no epoll_wait: " << strerror(errno) << std::endl;
			break;
		}
		
		epollReadyListLoop(sockets);
		epollValidationLoop();
	}
	
	std::cout << "[MAIN] Loop do servidor encerrado" << std::endl;
}

int main(int ac, char **av)
{
	// 1. Configurar handlers de sinais ANTES de inicializar o runtime
	SignalHandler::setupSignalHandlers();
	
	// 2. Inicializar runtime
	if (RunTime::createRuntime(ac, av) != 0)
	{
		std::cerr << "Erro ao inicializar runtime" << std::endl;
		return 1;
	}
	
	// 3. Executar loop principal
	try {
		serverLoop();
	}
	catch (std::exception &e) {
		std::cerr << "[ERROR] Exceção capturada: " << e.what() << std::endl;
		RunTime::gracefulShutdown();
		return 1;
	}
	
	// 4. Shutdown gracioso
	std::cout << "[MAIN] Iniciando shutdown gracioso..." << std::endl;
	RunTime::gracefulShutdown();
	
	std::cout << "[MAIN] Servidor encerrado com sucesso. Até logo! 👋" << std::endl;
	return 0;
}
