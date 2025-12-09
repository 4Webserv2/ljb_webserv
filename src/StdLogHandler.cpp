/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:10:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/09 12:50:20 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/StdLogHandler.hpp"

StdLogHandler::StdLogHandler() : LogHandler() {}

StdLogHandler::~StdLogHandler() {}

void StdLogHandler::handleDebug(t_event event) {
	std::cout << Logger::getTimestamp() << " [DEBUG] " << event.message << std::endl;
}

void StdLogHandler::handleInfo(t_event event) {
	std::cout << Logger::getTimestamp() << " [INFO] " << event.message << std::endl;
}

void StdLogHandler::handleWarning(t_event event) {
	std::cout << Logger::getTimestamp() << " [WARNING] " << event.message << std::endl;
}

void StdLogHandler::handleError(t_event event) {
	std::cout << Logger::getTimestamp() << " [ERROR] " << event.message << std::endl;
}
