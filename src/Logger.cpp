/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 17:07:18 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/02 14:01:39 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// debug, info, error e warning emitem eventos,
// logger filtra e strategy implementar

#include "../includes/Logger.hpp"

Logger *Logger::_instance = NULL;

Logger::Logger(enum LogLevel level, LogHandler *handler) : _handler(handler), _level(level) {}

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

std::string Logger::getTimestamp() {
	std::time_t t = std::time(NULL);
	std::string s = std::ctime(&t);

	long unsigned int pos = s.find("\n");
	if (pos != std::string::npos)
		s.erase(pos, 1);

	return s;
}
