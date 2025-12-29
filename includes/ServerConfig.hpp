#pragma once

#include "Webserv.hpp"

enum TypeValidation
{
	EMPTY = 0,
	SEMICOLON = 1,
	DIFF_SEMICOLON = 2,
	END_OF_FILE = 3
};

class ServerBlock;

class ConfigFile
{
	private:
		std::vector<std::string> _tokens;
		std::vector<ServerBlock> _serverBlocks;

	public:
		ConfigFile(void);
		~ConfigFile(void);
		ConfigFile(int ac, char **av);

		std::vector<std::string> tokenizeContent(const std::string &content);
		void parser(const std::string &filename);
		void removeTokens(size_t amount);
		void verifyToken(TypeValidation type, const std::string &message);

		static void readFile(const std::string &filename, std::string &content);
		static void trim(std::string &content);
		static void removeComments(std::string &content);
		static void cleanFile(const std::string &filename, std::string &content);

		std::vector<std::string> getTokens(void);
		const std::vector<ServerBlock> &getServerBlocks(void) const;

		void validateDuplicateListensAcrossServers() const;

		void initServerSockets(int socketDomain, int socketType);
};
