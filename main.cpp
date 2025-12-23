/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 16:51:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/23 19:23:57 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Webserv.hpp"
#include "includes/Runtime.hpp"
#include "includes/HttpRequest.hpp"
#include "includes/HttpResponse.hpp"
#include "includes/ServerManage.hpp"
#include "includes/SignalHandler.hpp"
#include "includes/Logger.hpp"
#include "includes/CompositeLogHandler.hpp"
#include "includes/StdLogHandler.hpp"
#include "includes/FileLogHandler.hpp"
#include "includes/StringUtils.hpp"

/*int	clientLoop(const int& clientFd) {
	char	buffer[BUFFER_SIZE];
	HttpRequest request;
	std::string rawRequest;
	int bytesRead;

	do
	{
		bytesRead = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
		if (bytesRead <= 0)
		{
			if (bytesRead == 0) {

				Logger::info("Client disconnected");
			}
			else {

				Logger::error("Error: client read");
				close(clientFd);
			}
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
}*/

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
		if (sockets == -1)
		{
			if (errno == EINTR) {
				if (SignalHandler::isShutdownRequested()) {
					SignalHandler::handleShutdownMessage();
					Logger::info("[MAIN] epoll_wait interrupted by shutdown signal");
					break;
				}
				continue;
			}
			StringUtils::errorAndCerr("[ERROR] Error in epoll_wait: " + std::string(strerror(errno)));
			break;
		}
		else
		{
			epollReadyListLoop(sockets);
			EpollInstance::deletePendingRemovals();
			epollValidationLoop();
		}
	}

	Logger::info("[MAIN] Server loop terminated");
}

bool checkArguments(int argc, char **argv) {
	if (argc > 2) {
		std::cerr << "Wrong number of arguments. It must be: " << argv[0] << " <config_file>\n\tOR\nUsage: " << argv[0] << std::endl;
		return (false);
	}
	return (true);
}

int main(int ac, char **av)
{
	if (!checkArguments(ac, av))
		return (1);

	CompositeLogHandler* compositeHandler = new CompositeLogHandler();
	compositeHandler->addHandler(new StdLogHandler());
	compositeHandler->addHandler(new FileLogHandler("app.log"));
	Logger::initializeLogger(DEBUG, compositeHandler);

	SignalHandler::setupSignalHandlers();

	try
	{
		RunTime::createRuntime(ac, av);
		serverLoop();
	}
	catch (const std::exception &e)
	{
		Logger::error(std::string("Caught exception: ") + e.what());
		RunTime::gracefulShutdown();
		Logger::shutdownLogger();

		return (1);
	}
	RunTime::gracefulShutdown();
	Logger::info("Server finished. Bye, see you! 👋");
	Logger::shutdownLogger();

	return (0);
}
