/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/03/18 17:08:16 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::initServer(std::string port)
{
	_pollfds = 0;
	_curFrame = 0;

	try
	{
		initServerSocket(stoi(port));
		std::cout << "server initialized with socket " << _serverSocket << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << "\033[31;1mserver initialization error : " << e.what() << "\033[0m" << std::endl;
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

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
    	perror("setsockopt");
		throw std::runtime_error("can't set socket options");
	}

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = INADDR_ANY;
	s_addr.sin_port = htons(port);
	if (bind(_serverSocket, (struct sockaddr *)&s_addr, \
	sizeof(struct sockaddr_in)) < 0)
	{
		std::perror("bind :");
		throw std::runtime_error("can't bind socket");
	}


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
	(void)write(fd, (uint8_t []){SET_MAP}, 1);
	(void)write(fd, _sceneName.c_str(), _sceneName.size() + 1);
	updatePollfds();
	return(1);
}

void Clusterizer::redistributeJob(t_job *job)
{
	size_t highestInProgress = 0;
	auto clientHighest = _clients.begin();
	t_job	*replaced;
	std::vector<t_job *>::iterator found;

	if(!_clients.size())
	{
		found = std::find(_jobs[IN_PROGRESS].begin(), _jobs[IN_PROGRESS].end(), job);
		_jobs[IN_PROGRESS].erase(found);
		_jobs[WAITING].insert(_jobs[WAITING].begin(), job);
	}
	
	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(!it->second.curJob)
		{
			highestInProgress = (size_t) -1;
			clientHighest = it;
		}
		else if(it->second.curJob->frameNb > highestInProgress)
		{
			highestInProgress = it->second.curJob->frameNb;
			clientHighest = it;
		}
	}

	if(clientHighest->second.curJob)
	{
		(void)write(clientHighest->first, (uint8_t []){ABORT}, 1);
		replaced = clientHighest->second.curJob;	
		found = std::find(_jobs[IN_PROGRESS].begin(), _jobs[IN_PROGRESS].end(), replaced);
		_jobs[IN_PROGRESS].erase(found);
		_jobs[WAITING].insert(_jobs[WAITING].begin(), clientHighest->second.curJob);
	}
	clientHighest->second.curJob = job;
	(void)write(clientHighest->first, (uint8_t []){JOB}, 1);
	(void)write(clientHighest->first, job, sizeof(t_job));
}

void Clusterizer::deleteClient(int fd)
{
	std::map<int, t_client>::iterator it;

	std::cout << "client disconnected" << std::endl;
	it = _clients.find(fd);
	if(it == _clients.end())
		return;
	if(_clients[fd].curJob)
		redistributeJob(_clients[fd].curJob);
	_clients.erase(it);
	updatePollfds();
	close(fd);
}

void Clusterizer::getImageFromClient(int fd, std::vector<uint8_t> &buf)
{
	std::cout << "client sent an image" << std::endl;


	buf.erase(buf.begin());

	if(_clients[fd].gotGo && _clients[fd].curJob)
		_renderer->addImageToRender(buf);
	else
	{
		std::cout << "client sent an image before receiving a ready signal, dropping image" << std::endl;
		buf.erase(buf.begin(), buf.begin() + (WIDTH * HEIGHT * 3));
		return;
	}

	buf.erase(buf.begin(), buf.begin() + (WIDTH * HEIGHT * 3));

	_clients[fd].gotGo = 0;
	_clients[fd].readyRespond = 0;

	auto posIter = std::find(_jobs[IN_PROGRESS].begin(), _jobs[IN_PROGRESS].end(), _clients[fd].curJob);
	_jobs[IN_PROGRESS].erase(posIter);
	_jobs[DONE].push_back(_clients[fd].curJob);

	_clients[fd].curJob = 0;
	_curFrame++;
}

bool Clusterizer::handleBuffer(int fd, std::vector<uint8_t> &buf)
{
	std::vector<uint8_t>	sendBuffer;
	uint8_t					tmp;

	if(buf[0] == RDY)
	{
		_clients[fd].ready = 1;
		buf.erase(buf.begin());
		std::cout << "client " << fd << " is ready !" << std::endl;
	}
	else if(buf[0] == IMG_SEND_RQ)
	{
		std::cout << "client " << fd << " is ready to send its image" << std::endl;
		_clients[fd].readyRespond = 1;
		buf.erase(buf.begin());
	}
	else if(buf[0] == PROGRESS_UPDATE)
	{
		if(buf.size() < 2)
			return (0);
		tmp = buf[1];
		std::cout << "client " << fd << " sent a progress update, new progress : " << (int)tmp << std::endl;
		buf.erase(buf.begin(), buf.begin() + 2);
		if (tmp <= 100)
			_clients[fd].progress = tmp;
	}
	else if(buf[0] == IMG)
	{
		if(buf.size() < ((WIDTH * HEIGHT  * 3) + 1))
		{
			std::cout << "incomplete IMG request, " << buf.size() << "/"<< (WIDTH * HEIGHT * 3) << std::endl;
			return (0);
		}

		getImageFromClient(fd, buf);
	}
	else
	{
		std::cout << "client sent an unknown request, ignoring" << std::endl;
		buf.erase(buf.begin());
	}

	if(sendBuffer.size())
		(void)write(fd, sendBuffer.data(), sendBuffer.size());
	return(buf.size() != 0);
}

int Clusterizer::updateBuffer(int fd)
{
	uint8_t buffer[512];
	size_t ret;

	ret = recv(fd, buffer, 512, 0);
	if(!ret || ret == (size_t)-1)
		return(1);
	_clients[fd].buffer.insert(_clients[fd].buffer.end(), buffer, buffer + ret);
	while(handleBuffer(fd, _clients[fd].buffer))
		;;
	return(0);
}

int Clusterizer::dispatchJobs(void)
{
	t_job	*tmp;
	int		dispatched;

	dispatched = 0;
	if(!_jobs[WAITING].size())
		return (0);

	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(it->second.ready && !it->second.curJob)
		{
			tmp = _jobs[WAITING].front();
			_jobs[WAITING].erase(_jobs[WAITING].begin());
			_jobs[IN_PROGRESS].push_back(tmp);
			(void)write(it->first, (uint8_t []){JOB}, 1);
			(void)write(it->first, tmp, sizeof(t_job));
			it->second.progress = 0;
			it->second.curJob = tmp;
			it->second.readyRespond = 0;
			it->second.gotGo = 0;
			dispatched = 1;
		}
		if(!_jobs[WAITING].size())
			return (1);
	}
	return (dispatched);
}

int Clusterizer::handleSendRequests(void)
{
	int action;

	action = 0;
	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(it->second.curJob && it->second.curJob->frameNb == _curFrame + 1 && it->second.readyRespond && !it->second.gotGo)
		{
			action = 1;
			(void)write(it->first, (uint8_t []){RDY}, 1);
			it->second.gotGo = 1;
		}
	}
	return(action);
}

void Clusterizer::addJob(glm::vec3 pos, glm::vec2 dir, size_t samples, size_t frame, GPUDenoise &denoise)
{
	t_job *tmp;

	tmp = new t_job;
	tmp->pos = pos;
	tmp->dir = dir;
	tmp->samples = samples;
	tmp->frameNb = frame;
	tmp->denoise = denoise;
	_jobs[WAITING].push_back(tmp);
	_curFrame = 0;

	std::cout << "new job added : " << std::endl;
	std::cout << "	- pos : " << glm::to_string(pos) << std::endl;
	std::cout << "	- dir : " << glm::to_string(dir) << std::endl;
	std::cout << std::endl;
}

void Clusterizer::abortJobs(void)
{
	for(auto it = _jobs[WAITING].begin(); it != _jobs[WAITING].end(); it++)
		delete *it;
	_jobs[WAITING].clear();

	for(auto it = _clients.begin(); it != _clients.end(); it++)
	{
		if(it->second.curJob)	
		{
			(void)write(it->first, (uint8_t []){ABORT}, 1);
			delete it->second.curJob;
			it->second.curJob = 0;
			it->second.progress = 0;
		}
	}
	_jobs[IN_PROGRESS].clear();

	for(auto it = _jobs[DONE].begin(); it != _jobs[DONE].end(); it++)
		delete *it;
	_jobs[DONE].clear();
}

void Clusterizer::updateServer(void)
{
	int recv;
	int	didSomething;

	if(!_jobs[WAITING].size() && !_jobs[IN_PROGRESS].size() && _jobs[DONE].size())
	{
		std::cout << "clusterized render done, closing output video" << std::endl;
		_renderer->endRender(this);
	}
	didSomething = 1;
	while(didSomething)
	{
		didSomething = acceptClients();
		if(handleSendRequests())
			didSomething = 1;
		if(dispatchJobs())
			didSomething = 1;
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
	}
}
