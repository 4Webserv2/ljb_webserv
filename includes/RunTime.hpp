/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:45:27 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/27 11:45:43 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class RunTime
{
	private:
		static RunTime              *_instance;
		ConfigFile                  _config;

		RunTime(void);
		RunTime(int ac, char **av);
		RunTime(const RunTime &src);
		RunTime &operator=(const RunTime &src);

	public:
		~RunTime(void);

		static void initializeRuntime(int ac, char **av);
		static void deleteInstance(void);

		static void initServerSockets(void);

		static RunTime &getInstance(void);
		static ConfigFile &getConfig(void);
};
