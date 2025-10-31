/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 17:07:18 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/31 07:46:23 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// debug, info, error e warning emitem eventos,
// logger filtra e strategy implementa

#include "../includes/Logger.hpp"

Logger *Logger::_instance = NULL;

Logger::Logger(enum LogLevel level, LogHandler *handler) : _level(level),
	_handler(handler) {}

Logger::~Logger() {}

int Logger::initializeLogger(enum LogLevel level, LogHandler *handler)
{
	if (Logger::_instance) {
		return (-1);
	}
	Logger::_instance = new Logger(level, handler);
	return (0);
}

void Logger::debug(std::string message)
{
	if (!Logger::_instance || !Logger::_instance->_handler
		|| Logger::_instance->_level > DEBUG) {
		return ;
	}
	t_event event = {
		.level = DEBUG,
		.message = message
	};
	Logger::_instance->_handler->handleDebug(event);
}

void Logger::info(std::string message)
{
	if (!Logger::_instance || !Logger::_instance->_handler
		|| Logger::_instance->_level > INFO) {
		return ;
	}
	t_event event = {
		.level = INFO,
		.message = message
	};
	Logger::_instance->_handler->handleInfo(event);
}

void Logger::warning(std::string message)
{
	if (!Logger::_instance || !Logger::_instance->_handler
		|| Logger::_instance->_level > WARNING) {
		return ;
	}
	t_event event = {.level = WARNING, .message = message};
	Logger::_instance->_handler->handleWarning(event);
}

void Logger::error(std::string message)
{
	if (!Logger::_instance || !Logger::_instance->_handler
		|| Logger::_instance->_level > ERROR) {
		return ;
	}

	t_event event = {
		.level = ERROR,
		.message = message
	};
	Logger::_instance->_handler->handleError(event);
}

// int main() {
//		Singleton* s1 = Singleton::getInstance();
//		s1->showMessage();

//		Singleton* s2 = Singleton::getInstance();
//		s2->showMessage();

//		if (s1 == s2) {
// std::cout << "s1 and s2 point to the same Singleton instance." << std::endl;
// Verify that both pointers refer to the same instance
// }
// }

// Formato da mensagem do log seria assim:
// [2024-01-22 10:49:14] INFO: Program started.
// [2024-01-22 10:49:14] DEBUG: Debugging information.
// [2024-01-22 10:49:14] ERROR: An error occurred.

// [2025-10-10 16:42:00] INFO: 127.0.0.1 GET /index.html 200
