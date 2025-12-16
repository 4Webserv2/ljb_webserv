/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CompositeLogHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 22:16:41 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/16 12:32:54 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CompositeLogHandler.hpp"

/**
 * @file CompositeLogHandler.hpp
 * @brief Defines the CompositeLogHandler class for broadcasting log events to multiple handlers.
 *
 * The CompositeLogHandler implements the LogHandler interface and maintains a
 * collection of child handlers. When an event is received, the composite forwards
 * that event to all attached handlers. This enables flexible and extensible
 * logging pipelines with no changes required in the Logger’s core logic.
 *
 * Key responsibilities:
 * - Store and manage any number of LogHandler instances.
 * - Forward log events (debug/info/warning/error) to all children.
 * - Allow dynamic composition of logging strategies at runtime.
 *
 * Usage:
 * Add any handler (e.g., ConsoleLogHandler, FileLogHandler, NetworkLogHandler)
 * using addHandler(), and the composite will fan-out log events automatically.
 *
 * This class is an application of the Composite design pattern, enabling
 * scalable and modular logging architectures.
 */


CompositeLogHandler::CompositeLogHandler() {}

CompositeLogHandler::~CompositeLogHandler() {
	for (size_t i = 0; i < _handlers.size(); i++) {
		delete _handlers[i];
	}
	_handlers.clear();
}

void CompositeLogHandler::addHandler(LogHandler *handler)
{
	_handlers.push_back(handler);
}

void CompositeLogHandler::handleDebug(t_event event)
{
	for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleDebug(event);
}

void CompositeLogHandler::handleInfo(t_event event)
{
	for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleInfo(event);
}

void CompositeLogHandler::handleWarning(t_event event)
{
	for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleWarning(event);
}

void CompositeLogHandler::handleError(t_event event)
{
	for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleError(event);
}
