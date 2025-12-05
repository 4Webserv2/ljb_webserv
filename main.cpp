/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/05 17:51:39 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"
#include "includes/Runtime.hpp"
#include "includes/HttpRequest.hpp"
#include "includes/HttpResponse.hpp"
#include "includes/ServerManage.hpp"
#include "includes/SignalHandler.hpp"
#include "includes/Logger.hpp"
#include "includes/StringUtils.hpp"

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
				Logger::info("Client disconnected");
			else
				Logger::error("Error: client read");
			close(clientFd);
			return (bytesRead == 0 ? NO_ERROR : E_ERROR);
		}
		buffer[bytesRead] = '\0';
		rawRequest += buffer;
		if (rawRequest.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	} while (bytesRead == BUFFER_SIZE - 1);
	Logger::info("Request received, processing...");
	try
	{

		request.setPar(request.httpParse(rawRequest));
		HttpResponse response;
		response = response.dispatchRequest(request);
		std::string responseStr = response.toString();
		send(clientFd, responseStr.c_str(), responseStr.length(), 0);
		Logger::info("Response sent successfully (status " + response.intToString(response.status_code) + ")");
	}
	catch (std::exception& e)
	{
		HttpResponse errorResponse;
		errorResponse.setErrorPage(400);
		std::string responseStr = errorResponse.toString();
		send(clientFd, responseStr.c_str(), responseStr.length(), 0);
		Logger::error(std::string("Error while processing request: ") + e.what());
	}
	close(clientFd);
	return NO_ERROR;
}

void epollValidationLoop()
{
	// Verificar se deve fazer shutdown
	if (SignalHandler::isShutdownRequested()) {
		Logger::info("[MAIN] Shutdown requested, stopping loop...");
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
	Logger::info("[MAIN] Server started. Press Ctrl+C to stop.");
	while (RunTime::isRunning() && !SignalHandler::isShutdownRequested())
	{
		int sockets = EpollInstance::manipEpollWait();

		// Se epoll_wait foi interrompido por sinal
		if (sockets == -1)
		{
			if (errno == EINTR) {
				// Interrompido por sinal, verificar se é shutdown
				if (SignalHandler::isShutdownRequested()) {
					Logger::info("[MAIN] epoll_wait interrupted by shutdown signal");
					break;
				}
				// Outro sinal, continuar
				continue;
			}

			std::cerr << "[ERROR] Error in epoll_wait: " << strerror(errno) << std::endl;
			break;
		}

		epollReadyListLoop(sockets);
		epollValidationLoop();
	}

	Logger::info("[MAIN] Server loop terminated");
}

int main(int ac, char **av)
{
	// 1. Configurar handlers de sinais ANTES de inicializar o runtime
	SignalHandler::setupSignalHandlers();
	Logger::initializeLogger(DEBUG, new StdLogHandler());

	// 2. Inicializar runtime
	if (RunTime::createRuntime(ac, av) != 0)
	{
		std::cerr << "Error initializing runtime" << std::endl;
		return 1;
	}

	// 3. Executar loop principal
	try {
		serverLoop();
	}
	catch (std::exception &e) {
		std::cerr << "[ERROR] Caught exception: " << e.what() << std::endl;
		RunTime::gracefulShutdown();
		return 1;
	}

	// 4. Shutdown gracioso
	Logger::info("Gracious Shutdown init..");
	RunTime::gracefulShutdown();

	Logger::info("[MAIN] Server finished with successfully. See you! 👋");
	return 0;
}
