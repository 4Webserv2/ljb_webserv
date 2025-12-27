/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:03:49 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/27 12:12:17 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STDLOGHANDLER_HPP
# define STDLOGHANDLER_HPP

# include "Logger.hpp"

/**
 * @file StdLogHandler.cpp
 * @brief Implements the standard output logging strategy.
 *
 * Each log level is printed to std::cout with a readable prefix. This class
 * provides a simple and direct logging backend suited for debugging.
 *
 * Responsibilities:
 *  - Format messages consistently
 *  - Write log events to stdout
 */

class StdLogHandler : public LogHandler
{
	public:
		StdLogHandler();
		virtual ~StdLogHandler();

		virtual void handleDebug(t_event event);
		virtual void handleInfo(t_event event);
		virtual void handleWarning(t_event event);
		virtual void handleError(t_event event);
};

#endif
