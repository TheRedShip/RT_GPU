/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/23 19:42:56 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

Renderer::Renderer(Scene *scene, Window *win)
{
	_scene = scene;
	_win = win;
	_min = 0;
	_sec = 0;
	_samples = 1;
	_curSamples = 0;
}

void Renderer::addPoint(void)
{
	t_pathPoint newPoint;
	Camera		*cam;
	std::vector<t_pathPoint>::iterator pos;

	cam = _scene->getCamera();
	newPoint.pos = cam->getPosition();	
	newPoint.dir = cam->getDirection();
	newPoint.time = _min + (_sec / 60);
	pos = _path.begin();
	while(pos != _path.end() && pos->time <= newPoint.time)
		pos++;
	_path.insert(pos, newPoint);
}

void Renderer::update(void)
{
	double			curTime;

	if(!_destPathIndex)
		return;
	curTime = glfwGetTime();
	_curSamples++;
	if(_curSamples == _samples)
	{
		makeMovement(curTime - _curSplitStart, curTime);
		_curSamples = 0;	
	}
}

void Renderer::makeMovement(float timeFromStart, float curSplitTimeReset)
{
	t_pathPoint		from;
	t_pathPoint		to;
	float			pathTime;
	Camera			*cam;
	glm::vec3		posStep;
	glm::vec2		dirStep;	

	from = _path[_curPathIndex];
	to = _path[_curPathIndex + 1];
	cam = _scene->getCamera();
	pathTime = (to.time - from.time) * 60;	

	posStep.x = ((to.pos.x - from.pos.x) / pathTime) * timeFromStart;
	posStep.y = ((to.pos.y - from.pos.y) / pathTime) * timeFromStart;
	posStep.z = ((to.pos.z - from.pos.z) / pathTime) * timeFromStart;
	dirStep.x = ((to.dir.x - from.dir.x) / pathTime) * timeFromStart;
	dirStep.y = ((to.dir.y - from.dir.y) / pathTime) * timeFromStart;

	if(timeFromStart >= pathTime)
	{
		posStep = to.pos - from.pos;
		dirStep = to.dir - from.dir;
		_curSplitStart = curSplitTimeReset;
		_curPathIndex++;
	}
	cam->setPosition(from.pos + posStep);
	cam->setDirection(from.dir.x + dirStep.x, from.dir.y + dirStep.y);
	_win->setFrameCount(0);
	if(_curPathIndex == _destPathIndex)
	{
		_destPathIndex = 0;
		std::cout << "done" << std::endl;
	}
}

void Renderer::renderImgui(void)
{
	ImGui::Begin("Renderer");

	ImGui::SliderInt("spi", &_samples, 1, 1000);
	ImGui::Separator();

	ImGui::SliderInt("minutes", &_min, 0, 2);
	ImGui::SliderFloat("seconds", &_sec, 0, 60);
	if(ImGui::Button("add step"))
		addPoint();
	ImGui::Separator();

	for(unsigned long i = 0; i < _path.size(); i++)
	{
		ImGui::Text("pos : %f, %f, %f",_path[i].pos.x, _path[i].pos.y, _path[i].pos.z);
		ImGui::Text("dir : %f, %f",_path[i].dir.x, _path[i].dir.y);
		ImGui::Text("time : %dm%ds", (int)_path[i].time, (int)(((_path[i].time - (int)_path[i].time)) * 60));

		if(ImGui::Button(("delete##" + std::to_string(i)).c_str()))
		{
			_path.erase(_path.begin() + i);
		}

		ImGui::SameLine();
		if(ImGui::Button(("go to pos##" + std::to_string(i)).c_str()))
		{
			_scene->getCamera()->setPosition(_path[i].pos);	
			_scene->getCamera()->setDirection(_path[i].dir.x, _path[i].dir.y);	
			_win->setFrameCount(-1);
		}

		ImGui::SameLine();
		if(ImGui::Button(("edit pos##" + std::to_string(i)).c_str()))
		{
			_path[i].pos = _scene->getCamera()->getPosition();	
			_path[i].dir = _scene->getCamera()->getDirection();
		}

		if(i)
			ImGui::SameLine();
		if(i && ImGui::Button(("test split##" + std::to_string(i)).c_str()))
		{
			_scene->getCamera()->setPosition(_path[i].pos);	
			_scene->getCamera()->setDirection(_path[i].dir.x, _path[i].dir.y);	
			_win->setFrameCount(-1);
			_curSplitStart = glfwGetTime();
			_curPathIndex = i - 1;
			_destPathIndex = i;
		}
		ImGui::Separator();
	}
	ImGui::End();
}
