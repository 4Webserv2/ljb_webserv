/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileLogHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 22:16:07 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/27 12:23:08 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILELOGHANDLER_HPP
#define FILELOGHANDLER_HPP

#include "Logger.hpp"
#include <fstream>

class FileLogHandler : public LogHandler
{
private:
	std::ofstream _file;

public:
	FileLogHandler(const std::string &filename);
	virtual ~FileLogHandler();

	virtual void handleDebug(t_event event);
	virtual void handleInfo(t_event event);
	virtual void handleWarning(t_event event);
	virtual void handleError(t_event event);
};

#endif
