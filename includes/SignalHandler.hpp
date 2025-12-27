/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:52:02 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/27 11:52:03 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class SignalHandler
{
	private:
		static volatile sig_atomic_t _shutdownRequested;
		static void signalCallback(int signum);

	public:
		static void setupSignalHandlers();
		static bool isShutdownRequested();
		static void requestShutdown();
		static void handleShutdownMessage();
};
