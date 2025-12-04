#include "../includes/SignalHandler.hpp"

volatile sig_atomic_t SignalHandler::_shutdownRequested = 0;

void SignalHandler::signalCallback(int signum) {
	const char* signalName = "UNKNOWN";
	
	if (signum == SIGINT)
		signalName = "SIGINT";
	else if (signum == SIGTERM)
		signalName = "SIGTERM";
	
	// Usar write() pois é async-signal-safe (printf não é)
	const char msg[] = "\n[SIGNAL] Recebido sinal de shutdown: ";
	write(STDERR_FILENO, msg, sizeof(msg) - 1);
	write(STDERR_FILENO, signalName, strlen(signalName));
	write(STDERR_FILENO, "\n", 1);
	
	_shutdownRequested = 1;
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
		std::cerr << "Erro ao registrar handler para SIGINT: " 
				<< strerror(errno) << std::endl;
	}
	
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		std::cerr << "Erro ao registrar handler para SIGTERM: " 
				<< strerror(errno) << std::endl;
	}
	
	// Ignorar SIGPIPE (conexões fechadas abruptamente)
	signal(SIGPIPE, SIG_IGN);
	
	std::cout << "[SIGNAL] Handlers configurados para SIGINT e SIGTERM" << std::endl;
}

bool SignalHandler::isShutdownRequested() {
	return _shutdownRequested != 0;
}

void SignalHandler::requestShutdown() {
	_shutdownRequested = 1;
}