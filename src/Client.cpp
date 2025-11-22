/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:24 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/21 15:38:11 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::~Client() {}

Client::Client(int clientFd, ServerManage &server): EpollHandler(EPOLLIN, clientFd, 30), _server(server)
{
	this->_state = 9;
	this->_rawRequest = "";
	this->_request = HttpRequest();
	this->_response = HttpResponse();
	this->_pendingResponse = "";
	this->_responseOffset = 0;
}

Client::Client(const Client &src): EpollHandler(src.getActiveEvents(), src.getSocketFd(), src.getEventsTimeout()), _server(src._server)
{
	*this = src;
}

Client &Client::operator=(const Client &src)
{
	if (this != &src)
	{
		this->_request = src._request;
		this->_response = src._response;
		this->_state = src._state;
		this->_pendingResponse = src._pendingResponse;
		this->_responseOffset = src._responseOffset;
	}
	return (*this);
}

// GETTERS

int Client::getState(void)
{
	return (this->_state);
}

std::string &Client::getRawRequest(void)
{
	return (this->_rawRequest);
}

HttpRequest &Client::getRequest(void)
{
	return (this->_request);
}

HttpResponse &Client::getResponse(void)
{
	return (this->_response);
}

void	Client::processRequest(){
	try{
		HttpParse req = this->_request.httpParse(this->_rawRequest);
		if(req.uri.empty() || req.uri[0] != '/'){
			this->_response.setErrorPage(404);
			return;
		}

		if(req.method != "GET" && req.method != "POST" && req.method != "DELETE"){
			this->_response.setErrorPage(405);
			return;
		}

		this->_response = this->_response.dispatchRequest(req);
	} catch(std::exception &error){
		this->_response.setErrorPage(400);
	}
}
