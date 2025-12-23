/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:18 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/22 20:48:03 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "EpollHandler.hpp"
# include "LocationBlock.hpp"

class ServerManage;

enum ClientState {
	STATE_READING_HEADER = 0,
	STATE_READING_BODY = 1,
	STATE_COMPLETE = 2
};

class Client: public EpollHandler
{
	private:
		int _state;
		std::string _rawRequest;
		ServerManage &_server;
		std::string _pendingResponse;
		size_t _responseOffset;

	public:
		HttpResponse	response;
		HttpRequest		request;

		~Client();
		Client(int clientFd, ServerManage &server);
		Client(const Client &src);
		Client &operator=(const Client &src);

		virtual void EpollInHandler(void);
		virtual void EpollOutHandler(void);
		virtual void deleteHandler(void);

		void concatenateRequestData(const std::string &data);
		bool isRequestComplete(void);
		bool sendResponse(const std::string &responseStr);
		void processRequest(void);

		int getState(void) const;
		std::string &getRawRequest(void);
		HttpRequest &getRequest(void);
		HttpResponse &getResponse(void);
		void setState(int state);
};
