/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 21:08:38 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 22:43:00 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::initServer(std::string port)
{
	try
	{
		initServerSocket(stoi(port));
	}
	catch(std::exception &e)
	{
		std::cerr << "server initialization error : " << e.what() << std::endl;
		_error = 1;
	}
}

void	Clusterizer::initServerSocket(uint16_t port)
{
	struct sockaddr_in	s_addr;

	_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("can't create socket");
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = INADDR_ANY;
	s_addr.sin_port = port >> 8 | port << 8;
	if (bind(_serverSocket, (struct sockaddr *)&s_addr, \
	sizeof(struct sockaddr_in)) < 0)
		throw std::runtime_error("can't bind socket");
	if (::listen(_serverSocket, 50) < 0)
		throw std::runtime_error("can't listen on socket");
}

void Clusterizer::updateServer(void)
{

}
