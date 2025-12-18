/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 17:00:17 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/18 17:13:58 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

/**
 * @file Logger.hpp
 * @brief Defines a basic logging system using Singleton and Strategy patterns.
 */

# include <iostream>
# include <string>
# include <ctime>
# include <ostream>

# define RESET_COLOR "\033[0m"
# define RED_COLOR "\033[31m"
# define GREEN_COLOR "\033[32m"
# define YELLOW_COLOR "\033[33m"
# define BLUE_COLOR "\033[34m"


/**
 * @enum LogLevel
 * @brief Defines the available logging severity levels.
 */

enum LogLevel
{
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

typedef struct s_event
{
	enum LogLevel level;
	std::string message;
} t_event;

/**
 * @class LogHandler
 * @brief Abstract interface for log consumers.
 *
 * Conventional order: DEBUG < INFO < WARNING < ERROR
 */
class LogHandler
{
  public:
	LogHandler() {}
	virtual ~LogHandler() {}

	virtual void handleDebug(t_event event) = 0;
	virtual void handleInfo(t_event event) = 0;
	virtual void handleWarning(t_event event) = 0;
	virtual void handleError(t_event event) = 0;
};

/**
 * @class Logger
 * @brief Singleton logger that delegates messages to a LogHandler.
 */
class Logger
{
  private:
	Logger(enum LogLevel level, LogHandler *handler);
	Logger(const Logger &);

	static Logger *_instance;
	LogHandler *_handler;
	enum LogLevel _level;

  public:
	~Logger();

	static int initializeLogger(enum LogLevel level, LogHandler *handler);
	static void shutdownLogger();
	static void debug(std::string message);
	static void info(std::string message);
	static void warning(std::string message);
	static void error(std::string message);
	static std::string getTimestamp();
};

#endif
