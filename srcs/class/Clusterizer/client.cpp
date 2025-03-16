/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/03/18 17:11:04 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					shaderDenoise(ShaderProgram &denoising_program, GPUDenoise &denoise, std::vector<GLuint> textures);

void Clusterizer::initClient(std::string &dest)
{
	_serverFd = 0;
	_currentJob = 0;
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

bool Clusterizer::stringComplete(void)
{
	for(size_t i = 1; i < _receiveBuffer.size(); i++)
		if(_receiveBuffer[i] == 0)
			return(true);
	return(false);	
}

void Clusterizer::changeMap(Scene &scene, std::vector<Buffer *> &buffers, Window &win)
{
	size_t len;

	len = 1;
	while(_receiveBuffer[len])
		len++;
	_sceneName = std::string((char *)_receiveBuffer.data() + 1);	
	_receiveBuffer.erase(_receiveBuffer.begin(), _receiveBuffer.begin() + len);
	std::cout << "changing scene to name :" << _sceneName << std::endl;
	scene.changeScene(_sceneName, buffers);
	win.setFrameCount(0);
	if(scene.error())
		throw std::runtime_error("map change failed");
	(void)write(_serverFd, (uint8_t []){RDY}, 1);
}

void Clusterizer::clientHandleBuffer(Scene &scene, std::vector<Buffer *> &buffers, Window &win)
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
	{
		std::cout << "server is ready to receive" << std::endl;
		_srvReady = 1;
		_receiveBuffer.erase(_receiveBuffer.begin());
	}
	else if(_receiveBuffer[0] == ABORT)
	{
		std::cout << "got a abort request, aborting current job" << std::endl;
		delete _currentJob;
		_currentJob = 0;
		_receiveBuffer.erase(_receiveBuffer.begin());
	}
	else if(_receiveBuffer[0] == SET_MAP)
	{
		std::cout << "got change map request" << std::endl;
		if(stringComplete())
			changeMap(scene, buffers, win);
		else
			std::cout << "string is not complete, waiting for eof signal" << std::endl;
	}
	else
	{
		std::cout << "unknown request sent, ignoring" << std::endl;
		_receiveBuffer.erase(_receiveBuffer.begin());
	}


	if(sendBuf.size())
		(void)write(_serverFd, sendBuf.data(), sendBuf.size());
}

void Clusterizer::clientReceive(Scene &scene, std::vector<Buffer *> &buffers, Window &win)
{
	uint8_t	buffer[512];
	size_t	ret;

	ret = recv(_serverFd, buffer, 512, MSG_DONTWAIT);
	if(ret != (size_t)-1)
	{
		if(!ret)
		{
			if(_currentJob)
				delete _currentJob;
			_currentJob = 0;

			close(_serverFd);
			_serverFd = 0;
			return ;
		}
		_receiveBuffer.insert(_receiveBuffer.end(), buffer, buffer + ret);
	}
	clientHandleBuffer(scene, buffers, win);
}

void Clusterizer::sendProgress(uint8_t progress)
{
	uint8_t buf[2];
	_progress = progress;

	buf[0] = PROGRESS_UPDATE;
	buf[1] = progress;
	(void)write(_serverFd, buf, 2);
}

std::vector<uint8_t>	Clusterizer::rgb32fToRgb24i(std::vector<float> &imageFloat)
{
	std::vector<uint8_t> buffer(WIDTH * HEIGHT * 3);
	size_t rgbaIndex;
	size_t rgbIndex;

	for(size_t y = 0; y < HEIGHT; y++)
	{
		for(size_t x = 0; x < WIDTH; x++)
		{
			rgbaIndex = (((HEIGHT - 1 - y) * WIDTH) + x) * 4;
			rgbIndex = ((y * WIDTH) + x) * 3;
			buffer[rgbIndex] = fmin(imageFloat[rgbaIndex], 1) * 254;
			buffer[rgbIndex + 1] = fmin(imageFloat[rgbaIndex + 1], 1) * 254;
			buffer[rgbIndex + 2] = fmin(imageFloat[rgbaIndex + 2], 1) * 254;
			rgbIndex += 3;
			rgbaIndex += 4;
		}
	}
	return(buffer);
}

void	Clusterizer::sendImageToServer(Scene &scene, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers, Window &win)
{
	std::vector<float> imageFloat(WIDTH * HEIGHT * 4);
	std::vector<uint8_t> buffer;

	_srvReady = 0;
	(void)write(_serverFd, (uint8_t []){IMG_SEND_RQ}, 1);
	while(!_srvReady)
	{
		clientReceive(scene, buffers, win);
		if(!_serverFd)
		{
			delete _currentJob;
			_currentJob = 0;
		}
		if(!_currentJob)
			return ;
		usleep(10000);
	}
	std::cout << "server ready" << std::endl;

	if(_currentJob->denoise.enabled)
		shaderDenoise(denoisingProgram, _currentJob->denoise, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, imageFloat.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	buffer = rgb32fToRgb24i(imageFloat);
	std::cout << "buffer size : " << buffer.size() << std::endl;
	(void)write(_serverFd, (uint8_t []){IMG}, 1);
	(void)write(_serverFd, buffer.data(), buffer.size());
	delete _currentJob;
	_currentJob = 0;
	std::cout << "image sent" << std::endl;
}

void Clusterizer::handleCurrentJob(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers)
{
	uint8_t progress;
	if(!_currentJob)	
		return ;

	if(scene.getCamera()->getPosition() != _currentJob->pos || scene.getCamera()->getDirection() != _currentJob->dir)
	{
		std::cout << "not at right place, moving" << std::endl;
		scene.getCamera()->setPosition(_currentJob->pos);
		scene.getCamera()->setDirection(_currentJob->dir.x, _currentJob->dir.y);
		win.setFrameCount(0);
		return ;
	}


	if((size_t)win.getFrameCount() <= _currentJob->samples)
	{
		progress = ((double)win.getFrameCount() / _currentJob->samples) * 100;
		if(progress != _progress)
		{
			std::cout << "new progress" << std::endl;
			sendProgress(progress);
		}
	}

	std::cout << "sample " << win.getFrameCount() << "/" << _currentJob->samples << std::endl;
	if((size_t)win.getFrameCount() > _currentJob->samples)
		win.setFrameCount(0);

	if((size_t)win.getFrameCount() == _currentJob->samples)
	{
		std::cout << "send request" << std::endl;
		sendImageToServer(scene, textures, denoisingProgram, buffers, win);
	}

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

void Clusterizer::updateClient(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers)
{
	if(!_serverFd)
	{
		std::cout << "server isn't connected, waiting for connection" << std::endl;
		while(!_serverFd)
		{
			openClientConnection(_serverIp.c_str(), _serverPort);
			usleep(10000);
		}
		std::cout << "server reconnected." << std::endl;
	}

	clientReceive(scene, buffers, win);
	handleCurrentJob(scene, win, textures, denoisingProgram, buffers);
}
