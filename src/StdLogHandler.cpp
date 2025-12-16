/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:10:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/16 15:01:04 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/StdLogHandler.hpp"

StdLogHandler::StdLogHandler() : LogHandler() {}

StdLogHandler::~StdLogHandler() {}

void StdLogHandler::handleDebug(t_event event) {
	std::cout << BLUE_COLOR << Logger::getTimestamp() << " [DEBUG] " << event.message << RESET_COLOR << std::endl;
}

void StdLogHandler::handleInfo(t_event event) {
	std::cout << GREEN_COLOR << Logger::getTimestamp() << " [INFO] " << event.message << RESET_COLOR << std::endl;
}

void StdLogHandler::handleWarning(t_event event) {
	std::cout << YELLOW_COLOR << Logger::getTimestamp() << " [WARNING] " << event.message << RESET_COLOR << std::endl;
}

void StdLogHandler::handleError(t_event event) {
	std::cout << RED_COLOR << Logger::getTimestamp() << " [ERROR] " << event.message << RESET_COLOR << std::endl;
}
