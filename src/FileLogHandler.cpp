/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileLogHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 22:17:45 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/09 12:50:31 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/FileLogHandler.hpp"

/**
 * @file FileLogHandler.hpp
 * @brief Defines the FileLogHandler class responsible for writing log events to a file.
 *
 * The FileLogHandler implements the LogHandler interface and provides a concrete
 * output strategy that writes logs to a filesystem target. This handler is useful
 * for persistent log storage, auditing, and debugging scenarios where terminal
 * output is insufficient or too volatile.
 *
 * Key responsibilities:
 * - Open and manage a log file output stream.
 * - Format and write log messages based on severity.
 * - Handle file-access or I/O issues gracefully.
 *
 * Usage:
 * Instantiate a FileLogHandler with the desired filename and register it in the
 * Logger or CompositeLogHandler to enable file-based logging.
 *
 * This handler is typically combined with ConsoleLogHandler using
 * CompositeLogHandler when logs need to be duplicated to multiple outputs.
 */


FileLogHandler::FileLogHandler(const std::string &filename)
{
	_file.open(filename.c_str(), std::ios::out | std::ios::app);
}

FileLogHandler::~FileLogHandler()
{
	if (_file.is_open())
		_file.close();
}

void FileLogHandler::handleDebug(t_event event)
{
	_file << Logger::getTimestamp() << " [DEBUG] " << event.message << std::endl;
}

void FileLogHandler::handleInfo(t_event event)
{
	_file << Logger::getTimestamp() << " [INFO] " << event.message << std::endl;
}

void FileLogHandler::handleWarning(t_event event)
{
	_file << Logger::getTimestamp() << " [WARNING] " << event.message << std::endl;
}

void FileLogHandler::handleError(t_event event)
{
	_file << Logger::getTimestamp() << " [ERROR] " << event.message << std::endl;
}
