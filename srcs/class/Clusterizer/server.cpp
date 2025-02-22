/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/21 22:19:20 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::initServer(std::string port)
{
	_pollfds = 0;

	try
	{
		initServerSocket(stoi(port));
		std::cout << "server initialized with socket " << _serverSocket << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << "server initialization error : " << e.what() << std::endl;
		_error = 1;
	}
}

void	Clusterizer::initServerSocket(int port)
{
	struct sockaddr_in	s_addr;

	if(port > 65535 || port < 0)
		throw std::runtime_error("invalid port");
	_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("can't create socket");
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = INADDR_ANY;
	s_addr.sin_port = htons(port);
	if (bind(_serverSocket, (struct sockaddr *)&s_addr, \
	sizeof(struct sockaddr_in)) < 0)
		throw std::runtime_error("can't bind socket");
	if (::listen(_serverSocket, 50) < 0)
		throw std::runtime_error("can't listen on socket");
	if(_serverSocket == -1)
		throw std::runtime_error("unknown error");
}

void	Clusterizer::updatePollfds(void)
{
	if(_pollfds)
		delete[] _pollfds;
	_pollfds = new struct pollfd[_clients.size()];
	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		_pollfds[std::distance(it, _clients.begin())].fd = it->first;
		_pollfds[std::distance(it, _clients.begin())].events = POLLIN;
		_pollfds[std::distance(it, _clients.begin())].revents = 0;
	}
}

void Clusterizer::acceptClients(void)
{
	int fd;

	fd = accept(_serverSocket, 0, 0);
	if (fd != -1) {
		std::cout << "new client :" << fd << std::endl;
		_clients[fd].ready = 0;
		_clients[fd].curJob = 0;
		updatePollfds();
	}
}

void Clusterizer::deleteClient(int fd)
{
	std::map<int, t_client>::iterator it;

	std::cout << "client disconnected" << std::endl;
	it = _clients.find(fd);
	if(it == _clients.end())
		return;
	_clients.erase(it);
	updatePollfds();
	close(fd);
}

void Clusterizer::handleBuffer(int fd, std::vector<uint8_t> &buf)
{
	std::vector<uint8_t> sendBuffer;

	if(buf[0] == RDY)
	{
		_clients[fd].ready = 1;
		sendBuffer.push_back(ACK);
	}

	if(sendBuffer.size())
		(void)write(fd, sendBuffer.data(), sendBuffer.size());
}

int Clusterizer::updateBuffer(int fd)
{
	uint8_t buffer[512];
	size_t ret;

	ret = recv(fd, buffer, 512, 0);
	if(!ret)
		return(1);
	_clients[fd].buffer.insert(_clients[fd].buffer.end(), buffer, buffer + ret);
	handleBuffer(fd, _clients[fd].buffer);
	return(0);
}

void Clusterizer::updateServer(void)
{
	int recv;

	acceptClients();
	recv = poll(_pollfds, _clients.size(), 1);
	if(!recv)
		return ;
	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(_pollfds[std::distance(it, _clients.begin())].revents & POLLIN)
		{
			if (updateBuffer(it->first))
				deleteClient(it->first);
		}
	}
}
