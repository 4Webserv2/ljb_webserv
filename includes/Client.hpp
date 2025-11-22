/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:22:54 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "ServerManage.hpp"
# include "EpollHandler.hpp"

class HttpResponse;
class HttpRequest;

class Client: public EpollHandler
{
	private:
		int _state;
		std::string _rawRequest;
		ServerManage &_server;
		std::string _pendingResponse;
		size_t _responseOffset;

	public:
		HttpResponse	_response;
		HttpRequest		_request;

		~Client();
		Client(int clientFd, ServerManage &server);
		Client(const Client &src);
		Client &operator=(const Client &src);

		//Getters
		int getState(void);
		std::string &getRawRequest(void);
		HttpRequest &getRequest(void);
		HttpResponse &getResponse(void);

		void	processRequest();
};
