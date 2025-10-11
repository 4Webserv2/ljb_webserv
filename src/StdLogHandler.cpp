/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:10:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/11 20:50:58 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/StdLogHandler.hpp"

StdLogHandler::StdLogHandler() : LogHandler() {}

StdLogHandler::~StdLogHandler() {}

void StdLogHandler::handleDebug(t_event event) {
	std::cout << "[DEBUG] " << event.message << std::endl;
}

void StdLogHandler::handleInfo(t_event event) {
	std::cout << "[INFO] " << event.message << std::endl;
}

void StdLogHandler::handleWarning(t_event event) {
	std::cout << "[WARNING] " << event.message << std::endl;
}

void StdLogHandler::handleError(t_event event) {
	std::cout << "[ERROR] " << event.message << std::endl;
}
