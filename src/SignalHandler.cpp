/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:51:37 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/28 23:05:59 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/SignalHandler.hpp"
#include "../includes/Logger.hpp"

volatile sig_atomic_t SignalHandler::_shutdownRequested = 0;

void SignalHandler::signalCallback(int signum)
{
	_shutdownRequested = signum;
}

void SignalHandler::setupSignalHandlers()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalCallback;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errorAndCerr("Error registering handler for SIGINT: " + std::string(strerror(errno)));
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		errorAndCerr("Error registering handler for SIGTERM: " + std::string(strerror(errno)));
	signal(SIGPIPE, SIG_IGN);
	Logger::info("Signal handlers set up successfully for SIGINT and SIGTERM.");
}

bool SignalHandler::isShutdownRequested()
{
	return (_shutdownRequested != 0);
}

void SignalHandler::requestShutdown()
{
	_shutdownRequested = 1;
}

void SignalHandler::handleShutdownMessage()
{
	if (_shutdownRequested)
	{
		std::string signalName = "UNKNOWN";

		if (_shutdownRequested == SIGINT)
			signalName = "SIGINT";
		else if (_shutdownRequested == SIGTERM)
			signalName = "SIGTERM";
		std::cout << "\n";
		Logger::info("[SIGNAL] Shutdown signal received: " + signalName);
	}
}
