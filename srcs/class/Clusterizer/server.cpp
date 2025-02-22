/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/22 23:36:42 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::initServer(std::string port)
{
	_pollfds = 0;
	_curId = 0;

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
		_pollfds[std::distance(_clients.begin(), it)].fd = it->first;
		_pollfds[std::distance(_clients.begin(), it)].events = POLLIN;
		_pollfds[std::distance(_clients.begin(), it)].revents = 0;
	}
}

int Clusterizer::acceptClients(void)
{
	int fd;

	fd = accept(_serverSocket, 0, 0);
	if (fd == -1)
		return(0);
	std::cout << "new client :" << fd << std::endl;
	_clients[fd].ready = 0;
	_clients[fd].curJob = 0;
	updatePollfds();
	return(1);
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
		std::cout << "client " << fd << " is ready !" << std::endl;
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
	if(!ret || ret == (size_t)-1)
		return(1);
	_clients[fd].buffer.insert(_clients[fd].buffer.end(), buffer, buffer + ret);
	handleBuffer(fd, _clients[fd].buffer);
	return(0);
}

int Clusterizer::dispatchJobs(void)
{
	t_job *tmp;
	int		dispatched;

	dispatched = 0;
	if(!_jobs[WAITING].size())
		return (0);

	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(it->second.ready)
		{
			tmp = _jobs[WAITING].front();
			_jobs[WAITING].erase(_jobs[WAITING].begin());
			_jobs[IN_PROGRESS].push_back(tmp);
			(void)write(it->first, &tmp, sizeof(t_job));
			it->second.ready = 0;
			it->second.progress = 0;
			it->second.curJob = tmp;
			dispatched = 1;
		}
	}
	return (dispatched);
}

void Clusterizer::addJob(std::string scene, glm::vec3 pos, glm::vec2 dir, size_t samples)
{
	t_job *tmp;

	tmp = new t_job;
	tmp->scene = scene;
	tmp->pos = pos;
	tmp->dir = dir;
	tmp->samples = samples;
	tmp->id = _curId++;
	_jobs[WAITING].push_back(tmp);
}

void Clusterizer::updateServer(void)
{
	int recv;
	int	didSomething;

	didSomething = 1;
	while(didSomething)
	{
		didSomething = acceptClients();
		recv = poll(_pollfds, _clients.size(), 1);
		if(!recv)
			return ;
		for(auto it = _clients.begin(); it != _clients.end(); it++)
		{
			if(_pollfds[std::distance(_clients.begin(), it)].revents & POLLIN)
			{
				didSomething = 1;
				if (updateBuffer(it->first))
				{
					deleteClient(it->first);
					break;
				}
			}
		}
		if(dispatchJobs())
			didSomething = 1;
	}
}
