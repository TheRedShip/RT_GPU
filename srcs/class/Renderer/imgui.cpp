/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   imgui.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 15:54:35 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 16:10:17 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

float Renderer::calcTime(glm::vec3 pos)
{
	float prevSpeed;
	float time;
	int index;

	prevSpeed = 0;
	if(_path.size() > 1)
	{
		index = _path.size() - 1;
		while(index >= 1 && _path[index].time == _path[index - 1].time)
			index--;
		prevSpeed = glm::distance(_path[index - 1].pos, _path[index].pos) / (_path[index].time - _path[index - 1].time);
	}

	if(_autoTime)
	{
		if(_path.size() > 1)
			time = _path[_path.size() - 1].time + (glm::distance(_path[_path.size() - 1].pos, pos) / prevSpeed);
		else
			time = (float)_path.size() / 60;
		if(std::isnan(time))
			time = _path[_path.size() - 1].time + (1.0f / 60);
		_min = time;
		_sec = (time - (int)time) * 60;
	}
	else if(_tp)
	{
		if(!_path.size())
			time = 0;
		else
			time = _path[_path.size() - 1].time;
		_min = time;
		_sec = (time - (int)time) * 60;
	}
	else
		time = (float)_min + ((float)_sec / 60);
	return time;
}

void Renderer::imguiPathNodeList(void)
{
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
			_testMode = 1;
		}
		ImGui::Separator();
	}
}

void Renderer::imguiPathCreation(void)
{
	float time;

	ImGui::SliderInt("test spi", &_testSamples, 1, 10);
	
	if(ImGui::Button("render settings"))
		_renderSettings = 1;
	if(_path.size() > 1 && ImGui::Button("try full path"))
	{
		_scene->getCamera()->setPosition(_path[0].pos);	
		_scene->getCamera()->setDirection(_path[0].dir.x, _path[0].dir.y);	
		_win->setFrameCount(-1);
		_curSplitStart = glfwGetTime();
		_curPathIndex = 0;
		_destPathIndex = _path.size() - 1;
		_testMode = 1;
	}

	ImGui::Separator();

	if(ImGui::SliderInt("minutes", &_min, 0, 2)) 
	{
			_autoTime = 0;
			_tp = 0;
	}
	if(ImGui::SliderInt("seconds", &_sec, 0, 60))
	{
			_autoTime = 0;
			_tp = 0;
	}
	time = calcTime(_scene->getCamera()->getPosition());

	if (ImGui::Checkbox("guess time automatically", &_autoTime))
		_tp = 0;

	if(_autoTime)
	{
		ImGui::SameLine();
		ImGui::Checkbox("auto tp", &_autoTP);
	}


	if (ImGui::Checkbox("tp", &_tp))
		_autoTime = 0;
	if(ImGui::Button("add step"))
		addPoint(time);


	ImGui::Separator();
	imguiPathNodeList();
}

void	Renderer::imguiRenderSettings(void)
{
	ImGui::SliderInt("render spi", &_samples, 1, 1000);
	ImGui::SliderInt("render fps", &_fps, 30, 120);
	ImGui::Combo("codec", &_codecIndex, _codecListStr.data(), _codecListStr.size());
	if(ImGui::Checkbox("show all codecs", &_ignoreUnavailableCodec))
		updateAvailableCodecs(_ignoreUnavailableCodec, (AVCodecID)0);
	if(ImGui::InputText("file name", _filenameBuffer, 512))
	{
		_outputFilename = _filenameBuffer;
		updateAvailableCodecs(_ignoreUnavailableCodec, (AVCodecID)0);
	}
	if(_path.size() > 1 && _codecList.size())
	{
		try
		{
			if(ImGui::Button("start render"))
				initRender();
			ImGui::SameLine();
			if(ImGui::Button("save path"))
				savePath();
		}
		catch(std::exception &e)
		{
			std::cerr << "\033[31m" << e.what() << "\033[0m" << std::endl;
		}
	}
	if(ImGui::Button("go back"))
		_renderSettings = 0;
}

void Renderer::showRenderInfo(int isImgui)
{
	std::ostringstream oss;
	long int totalFrames;
	float renderTime;
	float progress;
	float timeElapsed;
	float timeEst;

	totalFrames = (_path[_destPathIndex].time - _path[0].time) * 60 * _fps;
	renderTime = ((float)_frameCount / _fps) / 60;

	timeElapsed = glfwGetTime() - _renderStartTime;
	timeEst = timeElapsed / ((_frameCount * _samples) + _curSamples);
	timeEst *= (totalFrames * _samples) - ((_frameCount * _samples) + _curSamples);
	if(timeEst > 1e15)
		timeEst = 0;

	oss << std::fixed << std::setprecision(2);

	oss << "render in progress" << std::endl;
	oss << "samples per frame : " << _samples << std::endl;
	oss << "render fps : " << _fps << std::endl;
	oss << "total render time : ";
	oss << floatToTime((_path[_destPathIndex].time - _path[0].time) * 60).c_str();

	if(isImgui)
	{
		ImGui::Text("%s",oss.str().c_str());
		ImGui::Separator();
	}
	else
	{
		std::cout << "\033[2J\033[H";
		std::cout << oss.str() << std::endl;
		std::cout << "-----------------------" << std::endl;
	}
	oss.str("");
	oss.clear();

	oss << "Frames : " << _frameCount << " / " << totalFrames << std::endl;
	oss << "Frames (with accumulation) : " << (_frameCount * _samples) + _curSamples;
	oss << " / " << totalFrames * _samples << std::endl;
	oss << "Render time : " << (int)renderTime << "m";
	oss << (renderTime - (int)renderTime) * 60 << "s" << std::endl;
	oss << "elapsed time : " << floatToTime(timeElapsed) << std::endl;
	oss << "estimated time remaining :" <<  floatToTime(timeEst);
	if(_headless)
		oss << std::endl << "fps : " << _win->getFps();

	progress = ((float)_frameCount * _samples)  + _curSamples;
	progress /= (float)totalFrames * _samples;

	if(isImgui)
	{
		ImGui::Text("%s",oss.str().c_str());
		ImGui::ProgressBar(progress, ImVec2(0, 0));
		if(ImGui::Button("stop"))
		{
			_destPathIndex = 0;
			endRender();
		}
	}
	else
	{
		oss << std::endl << progress * 100 << "%";
		std::cout << oss.str() << std::endl;
	}
}

std::string	Renderer::floatToTime(double timef)
{
	std::string res;
	uint64_t time;
	uint64_t values[7];
	int firstValue;

	time = timef;
	values[0] = time / (3600 * 24 *365);
	time = time % (3600 * 24 * 365);
	values[1] = time / (3600 * 24 * 30);
	time = time % (3600 * 24 * 30);
	values[2] = time / (3600 * 24 * 7);
	time = time % (3600 * 24 * 7);
	values[3] = time / (3600 * 24);
	time = time % (3600 * 24);
	values[4] = time / 3600;
	time = time % 3600;
	values[5] = time / 60;
	time = time % 60;
	values[6] = time;

	firstValue = 0;
	while(firstValue < 6 && values[firstValue] == 0 )
		firstValue++;

	res = "";
	switch(firstValue)
	{
		case 0:
			res += std::to_string(values[0]) + "Y";
		case 1:
			res += std::to_string(values[1]) + "M";
		case 2:
			res += std::to_string(values[2]) + "W";
		case 3:
			res += std::to_string(values[3]) + "d";
		case 4:
			res += std::to_string(values[4]) + "h";
		case 5:
			res += std::to_string(values[5]) + "m";
		case 6:
			res += std::to_string(values[6]) + "s";
	}
	return(res);
}

void Renderer::renderImgui(void)
{
	if (ImGui::CollapsingHeader("Renderer"))
	{
		if(rendering())
			showRenderInfo(1);
		else if(_renderSettings)
			imguiRenderSettings();
		else
			imguiPathCreation();
	}
}
