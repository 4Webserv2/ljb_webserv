/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdLogHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lraggio <lraggio@student.42.rio>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/10 23:03:49 by lraggio           #+#    #+#             */
/*   Updated: 2025/10/11 00:12:49 by lraggio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STDLOGHANDLER_HPP
# define STDLOGHANDLER_HPP

# include "Logger.hpp"

class StdLogHandler : public LogHandler
{
  public:
	StdLogHandler();
	~StdLogHandler();
	virtual void handleDebug(t_event event);
	virtual void handleInfo(t_event event);
	virtual void handleWarning(t_event event);
	virtual void handleError(t_event event);
};

#endif
