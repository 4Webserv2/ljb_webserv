/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logerTester.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:43 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:39:44 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Logger.hpp"
#include "../includes/StdLogHandler.hpp"

int main() {
	Logger::initializeLogger(ERROR, new StdLogHandler());

	Logger::debug("Esse é de debug");
	Logger::info("Esse é de info");
	Logger::warning("Esse é de warning");
	Logger::error("Esse é de error");
}
