/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CompositeLogHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 22:14:00 by lraggio           #+#    #+#             */
/*   Updated: 2025/12/27 12:12:54 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMPOSITELOGHANDLER_HPP
# define COMPOSITELOGHANDLER_HPP


#include "Logger.hpp"
#include <vector>

class CompositeLogHandler : public LogHandler
{
	private:
		std::vector<LogHandler*> _handlers;

	public:
		CompositeLogHandler();
		virtual ~CompositeLogHandler();

		void addHandler(LogHandler *handler);

		virtual void handleDebug(t_event event);
		virtual void handleInfo(t_event event);
		virtual void handleWarning(t_event event);
		virtual void handleError(t_event event);
};

#endif
