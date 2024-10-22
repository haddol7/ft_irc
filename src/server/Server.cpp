#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include "Server.hpp"

Server* Server::mInstance = NULL;

Server::Server()
{
	std::cout << "Server Init" << std::endl;
}

Server* Server::GetServer()
{
	if (mInstance == NULL)
	{
		mInstance = new Server();
	}
	return (mInstance);
}

void Server::InitServer(const char *port)
{
	struct sockaddr_in	address;
	
	mSocket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	
	/*fcntl*/

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(atoi(port));
	if (bind(mSocket, reinterpret_cast<sockaddr *>(& address), sizeof(address)) == -1)
	{
		//error here
	}
	if (listen(mSocket, LISTEN_SIZE) == -1)
	{
		//error here
	}
	mEpfd = epoll_create(EPOLL_SIZE);
	mEpollEvents = new struct epoll_event[EPOLL_SIZE];
	controlClientEvent(mSocket, EPOLL_CTL_ADD, EPOLLIN);
}

void Server::ExecServerLoop(void)
{
	int	event_count;
	int	i_event;
	int	client_fd;
	//int epoll_mode;

	while (true)
	{
		event_count = epoll_wait(mEpfd, mEpollEvents, EPOLL_SIZE, -1);
		if (event_count == -1)
		{ 
			//error here
		}
		for (i_event = 0; i_event < event_count; i_event++)
		{
			client_fd = mEpollEvents[i_event].data.fd;
			//epoll_mode = mEpollEvents[i_event].events;
			if (client_fd == mSocket)
			{
				registerClient();
			}
			else
			{
				while (1)
				{
					if (receiveFromClient(client_fd) == -1)
						break ;
				}
			}
			/*
			else if (epoll_mode & EPOLLOUT)
			{
				sendToClient(client_fd);
			}
			*/
		}
	}
	close(mSocket);
	close(mEpfd);
}

void Server::registerClient()
{	
	socklen_t			address_size;
	int					socket;
	struct sockaddr_in	address;

	address_size = sizeof(address);
	socket = accept(mSocket, reinterpret_cast<sockaddr *>(& address), &address_size);
	
	controlClientEvent(socket, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
	mClientMap.insert(std::make_pair(socket, Client(socket, address)));

	//debug
	{
		std::cout << socket << " | " << ntohs(address.sin_port) << " connected."<< std::endl;
	}
}

int Server::receiveFromClient(const int client_fd)
{
	int	len_buf;

	memset(mBuffer, 0, sizeof(mBuffer));
	std::cout << "Before read" << std::endl;
	len_buf = recv(client_fd, mBuffer, BUF_SIZE, MSG_DONTWAIT);
	std::cout << "After read" << std::endl;
	if (len_buf == -1)
	{
		if (errno == EAGAIN)
		{
			std::cout << "Now End!!!" << std::endl;
			return (-1);
		}
	}
	else if (len_buf == 0)
	{
		unregisterClientSocket(client_fd, " closed.");
		return (-1);
	}
	else
	{
		sendToClient(client_fd);
	}

	return (0);
}

//TODO	Implimention client list for send
//		현재는 본인을 제외한 모든 클라이언트에 메시지를 전달하지만,
//		나중에 모드를 구현할 때 어떤 클라이언트에 전송할 지에 대한
//		컨테이너가 필요할 것 같습니다.
void Server::sendToClient(const int client_fd)
{
	int	len_buf;
	int	it_fd;

	//test code for broadcasting
	for (std::map<const int, Client>::iterator it = mClientMap.begin(); it != mClientMap.end(); ++it)
	{	
		it_fd = it->second.GetFd();
		if (it_fd == client_fd)
		{
			controlClientEvent(it_fd, EPOLL_CTL_MOD, EPOLLIN);
			continue;
		}
		//debug
		{
			char	fd[1];

			fd[0] = '0' + client_fd;
			send(it_fd, "Debug <", 8, MSG_DONTWAIT);
			send(it_fd, fd, 1, MSG_DONTWAIT);
			send(it_fd, "> ", 3, MSG_DONTWAIT);
		}
		len_buf = write(it_fd, mBuffer, strlen(mBuffer));
		if (len_buf == -1)
		{
			unregisterClientSocket(it_fd, "sendToClient : write"); 
		}
		else
		{
			controlClientEvent(it_fd, EPOLL_CTL_MOD, EPOLLIN);
		}
	}
	memset(mBuffer, 0, sizeof(mBuffer));
}

void Server::controlClientEvent(const int client_fd, const int epoll_mode, const int event_mode)
{
	struct epoll_event	event;

	event.events = event_mode;
	event.data.fd = client_fd;
	epoll_ctl(mEpfd, epoll_mode, client_fd, &event);
}

void Server::unregisterClientSocket(const int client_fd, const std::string& msg)
{
	std::cerr << client_fd << msg << std::endl;
	epoll_ctl(mEpfd, EPOLL_CTL_DEL, client_fd, NULL);
	mClientMap.erase(client_fd);
	close(client_fd);
}

Client*	Server::ReturnClientOrNull(const int fd)
{
	std::map<int, Client>::iterator it = mClientMap.find(fd);

	if (it == mClientMap.end())
	{
		return (NULL);
	}
	return (&(mClientMap.find(fd)->second));
}

Client*	Server::ReturnClientOrNull(const std::string& nick)
{
	std::map<const int, Client>::iterator it = mClientMap.begin();

	for (; it != mClientMap.end(); ++it)
	{
		if (it->second.GetNickName() == nick)
		{
			return (&(it->second));
		}
	}
	return (NULL);
}
	
