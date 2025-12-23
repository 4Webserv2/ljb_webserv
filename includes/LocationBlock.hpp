/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jbergfel <jbergfel@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:04 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/23 19:32:41 by jbergfel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"
# include "ServerConfig.hpp"

class ServerConfig;

class LocationBlock {
	private:
		ServerConfig                &_config;
		bool                        _autoIndex;
		bool                        _canUpload;
		std::string                 _uri;
		std::string                 _alias;
		std::string                 _return;
		std::string                 _uploadPath;
		std::vector<std::string>    _index;
		std::vector<std::string>    _cgiExtensions;
		std::vector<std::string>    _allowMethods;

	public:
		LocationBlock(ServerConfig &config);
		~LocationBlock();
		LocationBlock(const LocationBlock &src);
		LocationBlock &operator=(const LocationBlock &src);

		void addAutoIndex();
		void addCanUpload();
		void addUri();
		void addAlias();
		void addReturn();
		void addUploadPath();
		void addIndex();
		void addCgiExtensions();
		void addAllowMethods();
		void printLocationBlock();
		void addLocationBlock();

		//| Getters
		bool getAutoIndex() const;
		bool getCanUpload() const;
		std::string getUri() const;
		std::string getAlias() const;
		std::string getReturn() const;
		std::string getUploadPath() const;
		std::vector<std::string> getIndex() const;
		std::vector<std::string> getCgiExtensions() const;
		std::vector<std::string> getAllowMethods() const;
};
