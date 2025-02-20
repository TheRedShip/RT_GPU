/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 16:06:18 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"


Renderer::Renderer(Scene *scene, Window *win, Arguments &args)
{
	std::string *renderPath;

	init(scene, win);
	_headless = args.getHeadless();
	renderPath = args.getRenderPath();

	if(renderPath)
	{
		try
		{
			loadPath(*renderPath);
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
			_shouldClose = 1;
		}
	}

	try{
		if(_headless)
			initRender();
	}
	catch(std::exception &e)
	{
		std::cerr << "\033[31m" << e.what() << "\033[0m" << std::endl;
		if(_headless)
			_shouldClose = 1;
	}
}

void	Renderer::init(Scene *scene, Window *win)
{
	_scene = scene;
	_win = win;
	_min = 0;
	_sec = 0;
	_fps = 30;
	_tp = 0;
	_shouldClose = 0;
	_autoTime = 0;
	_samples = 1;
	_testSamples = 1;
	_curSamples = 0;
	_destPathIndex = 0;
	_frameCount = 0;
	_renderSettings = 0;
	_outputFilename = "output.avi";
	memcpy(_filenameBuffer, _outputFilename.c_str(), _outputFilename.length());
	_filenameBuffer[_outputFilename.length()] = 0;

	_rgb_frame = 0;
	_yuv_frame = 0;
	_format = 0;
	_codec_context = 0;
	_ignoreUnavailableCodec = 0;
	updateAvailableCodecs(_ignoreUnavailableCodec, (AVCodecID)0);
}

void	Renderer::addPoint(float time)
{
	t_pathPoint newPoint;
	Camera		*cam;
	std::vector<t_pathPoint>::iterator pos;

	cam = _scene->getCamera();
	newPoint.pos = cam->getPosition();	
	newPoint.dir = cam->getDirection();
	newPoint.time = time; 
	pos = _path.begin();
	while(pos != _path.end() && pos->time <= newPoint.time)
		pos++;
	_path.insert(pos, newPoint);
}

void		Renderer::update(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	double			curTime;

	if(!_destPathIndex)
		return;

	_curSamples++;
	if(_headless)
		showRenderInfo(0);

	if((_testMode && _curSamples < _testSamples) || (!_testMode && _curSamples < _samples))
		return;

	if(_testMode)
		curTime = glfwGetTime();
	else
		curTime = (1 / (double)_fps) * (double)_frameCount;

	if(!_testMode)
	{
		addImageToRender(textures, denoisingProgram);
		_frameCount++;
	}
	makeMovement(curTime - _curSplitStart, curTime);
	_curSamples = 0;

}

void		Renderer::addTeleport(glm::vec3 from_pos, glm::vec2 from_dir, glm::vec3 to_pos, glm::vec2 to_dir)
{
	t_pathPoint point;

	if(!_autoTP || !_path.size() || !_autoTime)
		return ;

	point.pos = from_pos;
	point.dir = from_dir;
	point.time = calcTime(from_pos);
	_path.push_back(point);
	
	point.pos = to_pos;
	point.dir = to_dir;
	_path.push_back(point);
}

bool	Renderer::shouldClose(void) const
{
	return(_shouldClose);
}

int	Renderer::rendering(void) const
{
	return(_destPathIndex != 0 && !_testMode);
}
