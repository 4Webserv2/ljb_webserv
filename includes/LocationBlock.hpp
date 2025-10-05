#pragma once

# include "Webserv.hpp"

class ServerConfig;

class LocationBlock {
	private:
		bool _autoIndex;
		bool _canUpload;
		std::string _uri;
		std::string _alias;
		std::string _return;
		std::string _uploadPath;
		std::vector<std::string> _index;
		std::vector<std::string> _cgiExtensions;
		std::vector<std::string> _allowMethods;

		void addAutoIndex(ServerConfig &config);
		void addCanUpload(ServerConfig &config);
		void addUri(ServerConfig &config);
		void addAlias(ServerConfig &config);
		void addReturn(ServerConfig &config);
		void addUploadPath(ServerConfig &config);
		void addIndex(ServerConfig &config);
		void addCgiExtensions(ServerConfig &config);
		void addAllowMethods(ServerConfig &config);

	public:
		LocationBlock();
		~LocationBlock();

		void printLocationBlock();
		void addLocationBlock(ServerConfig &config);

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
