/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:24 by jbergfel          #+#    #+#             */
/*   Updated: 2025/11/08 20:39:25 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::~Client() {}

Client::Client() {}

Client::Client(int clientFd)
{
	this->_state = 0;
	this->_clientFd = clientFd;
	this->_rawRequest = "";
	this->_request = HttpParser();
}

Client::Client(int clientFd, int state, HttpParser request, HttpResponse response)
{
	this->_clientFd = clientFd;
	this->_state = state;
	this->_request = request;
	this->_response = response;
}

Client::Client(const Client &src)
{
	*this = src;
}

Client &Client::operator=(const Client &src)
{
	if (this != &src)
	{
		this->_state = src._state;
		this->_request = src._request;
		this->_response = src._response;
	}
	return (*this);
}


// GETTERS

int Client::getState(void)
{
	return (this->_state);
}

int Client::getClientFd(void)
{
	return (this->_clientFd);
}

std::string &Client::getRawRequest(void)
{
	return (this->_rawRequest);
}

HttpParser &Client::getRequest(void)
{
	return (this->_request);
}

HttpResponse &Client::getResponse(void)
{
	return (this->_response);
}
