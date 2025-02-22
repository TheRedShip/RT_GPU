/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/22 22:50:10 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::initClient(std::string &dest)
{
	_serverFd = 0;
	if(dest.find(":") == std::string::npos)
		std::cerr << "Client Initialisation error : invalid ip provided format must be <ip>:<port>" << std::endl;
	_serverIp = dest.substr(0, dest.find(":"));
	_serverPort = stoi(dest.substr(dest.find(":") + 1));

	try
	{
		openClientConnection(_serverIp.c_str(), _serverPort);
	}
	catch(std::exception &e)
	{
		if(_error)
			std::cerr << "\033[31mClient initialisation error : " << e.what() << std::endl;
	}
}

void Clusterizer::openClientConnection(const char *ip, int port)
{
    struct sockaddr_in  serv_addr;
	uint8_t sendBuffer;

	if(port > 65535 || port < 0)
	{
		_error = 1;
		throw std::runtime_error("invalid port provided");
	}
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
	{
		_error = 1;
		throw std::runtime_error("can't create socket"); 
	}
    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    if(connect(_serverFd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)))
	{
		close(_serverFd);
		_serverFd = 0;
	}
	sendBuffer = RDY;
	(void)write(_serverFd, &sendBuffer, 1);
}

void Clusterizer::clientGetJob(void)
{
	if(_receiveBuffer.size() < sizeof(t_job) + 1)
		return ;
	
	_currentJob = *(t_job *)(_receiveBuffer.data() + 1);
	_receiveBuffer.erase(_receiveBuffer.begin(), _receiveBuffer.begin() + sizeof(t_job) + 1);
}

void Clusterizer::clientHandleBuffer(void)
{
	std::vector<uint8_t> sendBuf;

	if(_receiveBuffer[0] == JOB)
		clientGetJob();

	if(sendBuf.size())
		(void)write(1, sendBuf.data(), sendBuf.size());
}

void Clusterizer::clientReceive(void)
{
	uint8_t	buffer[512];
	size_t	ret;

	ret = recv(_serverFd, buffer, 512, MSG_DONTWAIT);
	if(ret == (size_t)-1)
		return;
	if(!ret)
	{
		close(_serverFd);
		_serverFd = 0;
		return ;
	}
	_receiveBuffer.insert(_receiveBuffer.end(), buffer, buffer + ret);
}

void Clusterizer::updateClient(void)
{
	if(!_serverFd)
	{
		std::cout << "server isn't connected, waiting for connection" << std::endl;
		while(!_serverFd)
			openClientConnection(_serverIp.c_str(), _serverPort);
		std::cout << "server reconnected." << std::endl;
	}

	clientReceive();
	if(_receiveBuffer.size())
		clientHandleBuffer();
}
