/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:10:24 by lraggio           #+#    #+#             */
/*   Updated: 2025/11/04 18:40:40 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/StdLogHandler.hpp"

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
