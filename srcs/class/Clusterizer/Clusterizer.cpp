/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:24:39 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 22:42:42 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

Clusterizer::Clusterizer(Arguments args)
{
	_isActive = 1;
	_isServer = 0;
	_error = 0;
	_serverSocket = 0;

	if(args.getBoolean("server"))
	{
		_isServer = 1;
		initServer(*args.getString("server"));
	}
	else if(args.getBoolean("client"))
	{
		_isServer = 0;
		_serverIp = *args.getString("client");
		initClient();
	}
	else
		_isActive = 0;
}

Clusterizer::~Clusterizer(void)
{
	if(_serverSocket)
		close(_serverSocket);
}

void Clusterizer::update(void)
{
	if(!_isActive)
		return ;
	if(_isServer)
		updateServer();
	else
		updateClient();
}

bool Clusterizer::getError(void)
{
	return(_error);
}
