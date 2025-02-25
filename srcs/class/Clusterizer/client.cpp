/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/25 01:48:47 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					shaderDenoise(ShaderProgram &denoising_program, GPUDenoise &denoise, std::vector<GLuint> textures);

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
	uint8_t *data;
	std::cout << "received job" << std::endl;
	if(_receiveBuffer.size() < sizeof(t_job) + 1)
		return ;
	
	data = _receiveBuffer.data();
	_currentJob = new t_job;
	*_currentJob = *(t_job *)(data + 1);
	std::cout << "delete length : " << std::distance(_receiveBuffer.begin(), _receiveBuffer.begin() + sizeof(t_job) + 1) << std::endl;
	_receiveBuffer.erase(_receiveBuffer.begin(), _receiveBuffer.begin() + sizeof(t_job) + 1);
	std::cout << "new size : " << _receiveBuffer.size() << std::endl;
	_progress = 0;
}

void Clusterizer::clientHandleBuffer(void)
{
	std::vector<uint8_t> sendBuf;

	if(!_receiveBuffer.size())
		return ;
	std::cout << (int)_receiveBuffer.size() << std::endl;
	std::cout << sizeof(t_job) + 1 << std::endl;
	std::cout << std::endl;
	if(_receiveBuffer[0] == JOB)
		clientGetJob();
	else if(_receiveBuffer[0] == RDY)
		_srvReady = 1;
	else
		_receiveBuffer.erase(_receiveBuffer.begin());


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
	clientHandleBuffer();
}

void Clusterizer::sendProgress(uint8_t progress)
{
	uint8_t buf[2];
	_progress = progress;

	buf[0] = PROGRESS_UPDATE;
	buf[1] = progress;
	(void)write(_serverFd, buf, 2);
}

void	Clusterizer::sendImageToServer(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	_srvReady = 0;
	std::vector<uint8_t> buffer(WIDTH * HEIGHT * 4);

	(void)write(_serverFd, (uint8_t []){IMG_SEND_RQ}, 1);
	while(!_srvReady)
	{
		clientReceive();
		usleep(10000);
	}

	if(_currentJob->denoise.enabled)
		shaderDenoise(denoisingProgram, _currentJob->denoise, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	(void)write(_serverFd, (uint8_t []){IMG}, 1);
	(void)write(_serverFd, buffer.data(), buffer.size());
}

void Clusterizer::handleCurrentJob(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	uint8_t progress;
	if(!_currentJob)	
		return ;

	if(scene.getCamera()->getPosition() != _currentJob->pos || scene.getCamera()->getDirection() != _currentJob->dir)
	{
		scene.getCamera()->setPosition(_currentJob->pos);
		scene.getCamera()->setDirection(_currentJob->dir.x, _currentJob->dir.y);
		win.setFrameCount(0);
		return ;
	}

	if((size_t)win.getFrameCount() < _currentJob->samples)
	{
		progress = ((double)win.getFrameCount() / _currentJob->samples) * 100;
		if(progress != _progress)
			sendProgress(progress);
	}

	if((size_t)win.getFrameCount() == _currentJob->samples)
	{
		sendImageToServer(textures, denoisingProgram);
	}
}

void Clusterizer::updateClient(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	if(!_serverFd)
	{
		std::cout << "server isn't connected, waiting for connection" << std::endl;
		while(!_serverFd)
			openClientConnection(_serverIp.c_str(), _serverPort);
		std::cout << "server reconnected." << std::endl;
	}

	clientReceive();
	handleCurrentJob(scene, win, textures, denoisingProgram);
}
