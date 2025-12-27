/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 11:42:19 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/27 11:42:20 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class CgiHandler; // Forward declaration
class ServerManage;

class Client : public EpollHandler {
	private:
		int             _state;
		std::string     _rawRequest;
		size_t          _responseOffset;   // Offset atual da resposta sendo enviada
		std::string     _pendingResponse;  // Resposta pendente de envio (caso send() parcial)
		ServerManage    &_serverManage;
		public:
		HttpRequest     request;
		HttpResponse    response;
		CgiHandler    *cgiHandler;
		bool            logged;

		Client(int clientFd, ServerManage &serverManage);
		Client(const Client &src);
		Client &operator=(const Client &src);
		~Client(void);

		virtual void handleEpollIn(void);
		virtual void handleEpollOut(void);  // Para envio assíncrono quando socket está pronto

		void concatenateRequestData(std::string data);
		bool isRequestComplete(void);
		bool sendResponse(const std::string &responseStr);  // Envia resposta com tratamento correto de erros

		int getState(void) const;
		std::string &getRawRequest(void);
		HttpRequest &getRequest(void);
		HttpResponse &getResponse(void);

		void setState(int state);

		std::string toString(void) const;
		// Metodo para validar antes de executar o POST, DELETE e GET
		bool validateMethodAllowed(LocationBlock &location);
		bool validatingUriWithLocation(ServerBlock &serverBlock, LocationBlock &location);
		bool validateGet(ServerBlock &serverBlock, LocationBlock &location);
		bool validatePost(ServerBlock &serverBlock, LocationBlock &location);
		bool validateDelete(ServerBlock &serverBlock, LocationBlock &location);
};
