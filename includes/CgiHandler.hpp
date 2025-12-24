/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/24 09:54:48 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/24 10:04:21 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "ServerBlock.hpp"
#include "LocationBlock.hpp"
#include "EpollHandler.hpp"
#include "EpollInstance.hpp"
#include "ServerManage.hpp"
#include "StringUtils.hpp"

class CgiHandler: public EpollHandler
{
	private:
		int _fdIn[2];
		int _fdOut[2];
		std::string _scriptPath;
		std::vector<std::string> _env;
		std::string _cgiOutput;
		pid_t _childPid;

		const HttpRequest &_request;
		const ServerBlock &_serverBlock;
		const LocationBlock &_location;

		std::string _requestBody;
		size_t _bytesWritten;

	public:
		int status;
		CgiHandler(
			const HttpRequest &request,
			const ServerBlock &serverBlock,
			const LocationBlock &location);
		~CgiHandler();

		std::vector<std::string> buildEnvironment();
		bool start();

		virtual void handleEpollIn();
		virtual void handleEpollOut();

		const std::string &getCgiOutput() const;
		pid_t getChildPid() const;

		static bool isCgiScript(const std::string &uri, const LocationBlock &location);
		std::string extractCgiScriptPath(const std::string &uri);

		std::string normalizeHeaderName(const std::string &header);

		std::string getInterpretterPath(const std::string &scriptPath);
};
