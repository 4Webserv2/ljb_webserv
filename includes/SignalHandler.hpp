#pragma once

#include "Webserv.hpp"

class SignalHandler {
	private:
		static volatile sig_atomic_t _shutdownRequested;
		static void signalCallback(int signum);

	public:
		static void setupSignalHandlers();
		static bool isShutdownRequested();
		static void requestShutdown();
};