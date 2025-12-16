#include "../includes/SignalHandler.hpp"
#include "../includes/Logger.hpp"
#include "../includes/StringUtils.hpp"

volatile sig_atomic_t SignalHandler::_shutdownRequested = 0;

void SignalHandler::signalCallback(int signum) {
	 _shutdownRequested = signum;
}

void SignalHandler::setupSignalHandlers() {
	struct sigaction sa;

	// Zerar a estrutura
	memset(&sa, 0, sizeof(sa));

	// Configurar o handler
	sa.sa_handler = signalCallback;

	// Bloquear outros sinais durante a execução do handler
	sigemptyset(&sa.sa_mask);

	// SA_RESTART: reiniciar syscalls interrompidas automaticamente
	sa.sa_flags = SA_RESTART;

	// Registrar handlers para SIGINT e SIGTERM
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		StringUtils::errorAndCerr("Error registering handler for SIGINT: " + std::string(strerror(errno)));
	}

	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		StringUtils::errorAndCerr("Error registering handler for SIGTERM: " + std::string(strerror(errno)));
	}

	// Ignorar SIGPIPE (conexões fechadas abruptamente)
	signal(SIGPIPE, SIG_IGN);
	Logger::info("Signal handlers set up successfully for SIGINT and SIGTERM.");
}

bool SignalHandler::isShutdownRequested() {
	return _shutdownRequested != 0;
}

void SignalHandler::requestShutdown() {
	_shutdownRequested = 1;
}

void SignalHandler::handleShutdownMessage() {
	if (_shutdownRequested) {
		std::string signalName = "UNKNOWN";

		if (_shutdownRequested == SIGINT) {
			signalName = "SIGINT";
		}
		else if (_shutdownRequested == SIGTERM) {
			signalName = "SIGTERM";
		}
		Logger::info("[SIGNAL] Shutdown signal received: " + signalName);
	}
}
