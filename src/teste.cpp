#include "../includes/Webserv.hpp"
#include "../includes/Client.hpp"

int MAX_EVENTS = 10;

int make_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

const char http_response[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 13\r\n"
	"\r\n"
	"Bro-cha da VR!";

int create_server_socket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	make_nonblocking(sockfd);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, SOMAXCONN) == -1)
	{
		perror("listen");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

void handle_new_connection(int server_fd, int epfd, Client &client)
{
	client = accept(server_fd, NULL, NULL);
	if (client.getClientFd() == -1)
	{
		perror("accept");
		return;
	}

	make_nonblocking(client.getClientFd());

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = client.getClientFd();
	epoll_ctl(epfd, EPOLL_CTL_ADD, client.getClientFd(), &ev);

	std::cout << "Novo cliente conectado (fd=" << client.getClientFd() << ")\n";
}

void handle_client_request(int client_fd)
{
	char buffer[1024];
	int count = recv(client_fd, buffer, sizeof(buffer), 0);

	if (count <= 0)
	{
		std::cout << "Cliente desconectado (fd=" << client_fd << ")\n";
		close(client_fd);
	}
	else
		send(client_fd, http_response, sizeof(http_response) - 1, MSG_NOSIGNAL);
}

int test()
{
	Client client;

	int server_fd = create_server_socket();

	int epfd = epoll_create1(0);
	if (epfd == -1)
	{
		perror("epoll_create1");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	struct epoll_event events[10];
	memset(&ev, 0, sizeof(ev));
	memset(events, 0, sizeof(events));
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

	std::cout << "Servidor rodando em http://localhost:" << PORT << "\n";
	while (true)
	{
		int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (n == -1)
		{
			perror("epoll_wait");
			break;
		}

		for (int i = 0; i < n; i++)
		{
			if (events[i].data.fd == server_fd)
				handle_new_connection(server_fd, epfd, client);
			else if (events[i].events & EPOLLIN)
				handle_client_request(events[i].data.fd);
		}
	}
	close(server_fd);
	close(epfd);
	return 0;
}
