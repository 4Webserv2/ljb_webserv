/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Runtime.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 18:32:41 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/04 10:10:40 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Runtime.hpp"
#include "../includes/ServerManage.hpp"

RunTime *RunTime::_runtime = NULL;

RunTime::~RunTime(void) {}

RunTime::RunTime(void) : _running(true) {}

RunTime::RunTime(int ac, char **av): _config(ac, av), _running(true) {}

RunTime::RunTime(const RunTime &src)
{
	*this = src;
}

RunTime &RunTime::operator=(const RunTime &src)
{
	if (this != &src)
		this->_config = src._config;
	return (*this);
}

int RunTime::createRuntime(int ac, char **av)
{
	if (_runtime == NULL)
	{
		_runtime = new RunTime(ac, av);
		EpollInstance::initEpollRun();
		_runtime->initListeners();
		_runtime->initSockets(AF_INET, SOCK_STREAM);
		std::cout << "Runtime Initiated" << std::endl;
		return (0);
	}
	return (1);
}

void RunTime::initListeners(void)
{
	std::cout << "Loading server listeners..." << std::endl;
	std::set<std::pair<unsigned int, int> > uniqueListens;
	for (size_t i = 0; i < _runtime->_config.getServerBlocks().size(); i++)
	{
		std::cout << "Loading server block " << i + 1 << "..." << std::endl;
		std::vector<t_listen> listens = _runtime->_config.getServerBlocks()[i].getListen();

		for (size_t j = 0; j < listens.size(); j++)
		{
			std::pair<unsigned int, int> key(listens[j].host, listens[j].port);
			if (uniqueListens.insert(key).second)
				_runtime->_sListeners.push_back(ServerManage(listens[j].host, listens[j].port, _runtime->_config.getServerBlocks()[i]));
		}
	}
}

void RunTime::initSockets(int domain, int type)
{
	for (unsigned int i = 0; i < _runtime->_sListeners.size(); i++)
	{
		_runtime->_sListeners[i].startSocket(domain, type);
		EpollInstance::manipInterestList(EPOLL_CTL_ADD, &_runtime->_sListeners[i]);
	}
}

void RunTime::destroyRuntime(void)
{
	if (_runtime != NULL)
	{
		delete _runtime;
		_runtime = NULL;
	}
}

RunTime &RunTime::getRuntime(void)
{
	return (*_runtime);
}

ServerConfig &RunTime::getServerConfig(void)
{
	return (_runtime->_config);
}

std::vector<ServerManage> &RunTime::getListeners(void)
{
	return (_runtime->_sListeners);
}

std::map<int, Client> &RunTime::getClients(void)
{
	return (_runtime->_clients);
}

Client &RunTime::getClient(int client_fd)
{
	std::map<int, Client>::iterator clients = _runtime->_clients.find(client_fd);
	if (clients == _runtime->_clients.end())
	{
		std::cout << "Cannot find client!" << std::endl;
		return (clients->second);
	}
	return (clients->second);
}

ServerManage &RunTime::getElementInServerList(int serverSocketFd)
{
	for (size_t i = 0; i < _runtime->_sListeners.size(); i++)
	{
		if (_runtime->_sListeners[i].getSocketFd() == serverSocketFd)
			return (_runtime->_sListeners[i]);
	}
	return (_runtime->_sListeners[0]);;
}

void RunTime::deleteClient(int client_fd)
{
	if (_runtime == NULL)
		return;

	std::map<int, Client>::iterator it = _runtime->_clients.find(client_fd);
	if (it == _runtime->_clients.end())
		return;
	epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	_runtime->_clients.erase(it);
}

bool RunTime::isRunning() {
    if (_runtime == NULL)
        return false;
    return _runtime->_running;
}

void RunTime::setRunning(bool running) {
    if (_runtime != NULL)
        _runtime->_running = running;
}

void RunTime::closeAllClients() {
    if (_runtime == NULL)
        return;
    
    std::cout << "[SHUTDOWN] Fechando " << _runtime->_clients.size() 
              << " conexões de clientes..." << std::endl;
    
    // Copiar as chaves para evitar invalidação de iterador
    std::vector<int> clientFds;
    for (std::map<int, Client>::iterator it = _runtime->_clients.begin();
         it != _runtime->_clients.end(); ++it) {
        clientFds.push_back(it->first);
    }
    
    // Fechar cada cliente
    for (size_t i = 0; i < clientFds.size(); i++) {
        int fd = clientFds[i];
        
        // Enviar mensagem de encerramento se houver resposta pendente
        std::map<int, Client>::iterator it = _runtime->_clients.find(fd);
        if (it != _runtime->_clients.end()) {
            // Tentar enviar resposta final
            const char shutdownMsg[] = 
                "HTTP/1.1 503 Service Unavailable\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 79\r\n"
                "\r\n"
                "<html><body><h1>503 Service Unavailable</h1>"
                "<p>Server shutting down</p></body></html>";
            
            send(fd, shutdownMsg, sizeof(shutdownMsg) - 1, MSG_NOSIGNAL);
        }
        
        // Remover do epoll
        epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
        
        // Fechar socket
        close(fd);
        
        std::cout << "[SHUTDOWN] Cliente fd=" << fd << " fechado" << std::endl;
    }
    
    // Limpar mapa de clientes
    _runtime->_clients.clear();
    
    std::cout << "[SHUTDOWN] Todos os clientes foram desconectados" << std::endl;
}

void RunTime::closeAllServers() {
    if (_runtime == NULL)
        return;
    
    std::cout << "[SHUTDOWN] Fechando " << _runtime->_sListeners.size() 
              << " sockets de servidor..." << std::endl;
    
    for (size_t i = 0; i < _runtime->_sListeners.size(); i++) {
        int fd = _runtime->_sListeners[i].getSocketFd();
        
        // Remover do epoll
        epoll_ctl(EpollInstance::getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
        
        // Fechar socket
        close(fd);
        
        std::cout << "[SHUTDOWN] Servidor fd=" << fd 
                  << " (porta " << _runtime->_sListeners[i].getPort() 
                  << ") fechado" << std::endl;
    }
    
    _runtime->_sListeners.clear();
    
    std::cout << "[SHUTDOWN] Todos os servidores foram fechados" << std::endl;
}

void RunTime::gracefulShutdown() {
    if (_runtime == NULL)
        return;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "[SHUTDOWN] Iniciando encerramento gracioso..." << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Marcar como não mais em execução
    _runtime->_running = false;
    
    // 1. Parar de aceitar novas conexões (fechar listeners)
    closeAllServers();
    
    // 2. Fechar conexões de clientes existentes
    closeAllClients();
    
    // 3. Fechar epoll
    std::cout << "[SHUTDOWN] Fechando epoll..." << std::endl;
    close(EpollInstance::getEpollFd());
    
    // 4. Destruir runtime
    std::cout << "[SHUTDOWN] Liberando recursos do runtime..." << std::endl;
    destroyRuntime();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "[SHUTDOWN] Encerramento concluído com sucesso!" << std::endl;
    std::cout << "========================================\n" << std::endl;
}


