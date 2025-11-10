/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:39:19 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "HttpParser.hpp"
# include "HttpResponse.hpp"

class HttpResponse;
class HttpParser;

class Client
{
	private:
		std::string	_rawRequest;
		int	_clientFd;
		int	_state;

	public:
		HttpResponse	_response;
		HttpParser		_request;

		~Client();
		Client();
		Client(int clientFd);
		Client(int clientFd, int state, HttpParser request, HttpResponse response);
		Client(const Client &src);
		Client &operator=(const Client &src);

		//Getters
		int getState(void);
		int getClientFd(void);
		std::string &getRawRequest(void);
		HttpParser &getRequest(void);
		HttpResponse &getResponse(void);

		void	processRequest();
};
