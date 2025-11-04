/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 17:00:17 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/11 20:45:54 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>

enum				LogLevel
{
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

typedef struct s_event
{
	enum LogLevel	level;
	std::string message;
}					t_event;

// Define o que a classe concreta tem que saber consumir
// "Eu não me importo como vc vai fazer, eu só quero que vc saiba consumir
// um evento de debug, de info, de warn ou de erro."
class LogHandler
{
  public:
	LogHandler() = default;
	virtual ~LogHandler() = default;
	virtual void handleDebug(t_event event) = 0;
	virtual void handleInfo(t_event event) = 0;
	virtual void handleWarning(t_event event) = 0;
	virtual void handleError(t_event event) = 0;
};

class Logger
{
  private:
	Logger(enum LogLevel level, LogHandler *handler);
	Logger(const Logger &) = delete;
	Logger &operator=(const Logger &) = delete;

	static Logger *_instance;
	LogHandler *_handler;
	enum LogLevel _level;

  public:
	~Logger();
	static int initializeLogger(enum LogLevel level, LogHandler *handler);
	static void debug(std::string message);
	static void info(std::string message);
	static void warning(std::string message);
	static void error(std::string message);
};

#endif
