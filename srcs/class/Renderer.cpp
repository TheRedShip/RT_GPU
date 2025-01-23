/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/23 00:54:01 by tomoron          ###   ########.fr       */
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
	std::cout << "position : " <<  glm::to_string(newPoint.pos) << std::endl;
	std::cout << "direction : " <<  glm::to_string(newPoint.dir) << std::endl;
	std::cout << "time : " << newPoint.time << std::endl;
	pos = _path.begin();
	while(pos != _path.end() && pos->time <= newPoint.time)
		pos++;
	_path.insert(pos, newPoint);
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
			_scene->getCamera()->updateCameraVectors();
			_win->setFrameCount(-1);
		}
		ImGui::SameLine();
		if(ImGui::Button(("edit pos##" + std::to_string(i)).c_str()))
		{
			_path[i].pos = _scene->getCamera()->getPosition();	
			_path[i].dir = _scene->getCamera()->getDirection();
		}
		ImGui::Separator();
	}
	ImGui::End();
}
