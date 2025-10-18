#include "../include/Logger.hpp"
#include "../include/StdLogHandler.hpp"

int main() {
	Logger::initializeLogger(ERROR, new StdLogHandler());

	Logger::debug("Esse é de debug");
	Logger::info("Esse é de info");
	Logger::warning("Esse é de warning");
	Logger::error("Esse é de error");
}
