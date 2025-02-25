/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:24:39 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/25 22:33:01 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

Clusterizer::Clusterizer(Arguments &args, Renderer *renderer)
{
	_isActive = 1;
	_isServer = 0;
	_error = 0;
	_serverSocket = 0;
	_sceneName = args.getSceneName();
	_renderer = renderer;

	if(args.getBoolean("server"))
	{
		_isServer = 1;
		initServer(*args.getString("server"));
	}
	else if(args.getBoolean("client"))
	{
		_isServer = 0;
		initClient(*args.getString("client"));
	}
	else
		_isActive = 0;
}

Clusterizer::~Clusterizer(void)
{
	if(_serverSocket)
		close(_serverSocket);
}

void Clusterizer::update(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	if(!_isActive)
		return ;
	if(_isServer)
		updateServer();
	else
		updateClient(scene, win, textures, denoisingProgram);
}

bool Clusterizer::getError(void)
{
	return(_error);
}

bool Clusterizer::isServer(void)
{
	return(_isServer);	
}

bool Clusterizer::hasJobs(void)
{
	return(_jobs[WAITING].size() || _jobs[IN_PROGRESS].size());
}
