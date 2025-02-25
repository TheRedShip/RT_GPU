/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/25 19:16:53 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

//TODO: known problems : first image is put before starting the render and two first images are the same

Renderer::Renderer(Scene *scene, Window *win, Arguments &args)
{
	std::string *renderPath;

	init(scene, win);
	_headless = args.getBoolean("headless");
	renderPath = args.getString("renderpath");

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
			initRender(0);
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

	_ignoreUnavailableCodec = 0;
	Ffmpeg::updateAvailableCodecs(_codecList, _codecListStr, _outputFilename, _ignoreUnavailableCodec, (AVCodecID)0);
	_codecIndex = 0;
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
		showRenderInfo(0, 0);

	if((_testMode && _curSamples < _testSamples) || (!_testMode && _curSamples < _samples))
		return;

	if(_testMode)
		curTime = glfwGetTime() - _testStartTime;
	else
		curTime = (1 / (double)_fps) * (double)_frameCount;

	if(!_testMode)
	{
		addImageToRender(textures, denoisingProgram);
		_frameCount++;
	}
	makeMovement(curTime);
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

void	Renderer::createClusterJobs(Clusterizer &clust)
{
	double delta;
	size_t frames;
	glm::vec3 pos;
	glm::vec2 dir;

	delta = 1/(double)_fps;
	frames = 0;

	while(_destPathIndex)
	{
		interpolateMovement(delta * frames, &pos, &dir);
		frames++;
		clust.addJob(pos, dir, _samples, frames, _scene->getDenoise());
		if(_curPathIndex == _destPathIndex)
			_destPathIndex = 0;
		(void)clust;
	}
}

void	Renderer::initRender(Clusterizer *clust)
{
	if(_path.size() < 2)
		throw std::runtime_error("render path doesn't have enough path points");
	if(_path[0].time != 0)
		throw std::runtime_error("render path does not start at 0, aborting");
	if(_path[_path.size() - 1].time - _path[0].time <= 0)
		throw std::runtime_error("render path is 0 seconds long, aborting");

	_curPathIndex = 0;
	_frameCount = 0;
	_destPathIndex = _path.size() - 1;
	_renderStartTime = glfwGetTime();
	if(clust && clust->isServer())
		createClusterJobs(*clust);
	else
	{
		_curSamples = 0;
		_testMode = 0;
		_scene->getCamera()->setPosition(_path[0].pos);
		_scene->getCamera()->setDirection(_path[0].dir.x, _path[0].dir.y);
		_win->setFrameCount(_headless ? 0 : -1);
	}
	_ffmpegVideo = new Ffmpeg(_outputFilename, _fps, _codecList[_codecIndex]);
}

void		Renderer::addImageToRender(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	if(!_ffmpegVideo)
		return;
	_ffmpegVideo->addImageToVideo(*_scene, textures, denoisingProgram);
}

void		Renderer::addImageToRender(std::vector<uint8_t> &buf)
{
	if(!_ffmpegVideo)
		return;
	_ffmpegVideo->addImageToVideo(buf);
	_frameCount++;
}

void Renderer::endRender(Clusterizer *clust)
{
	_destPathIndex = 0;
	if(_headless)
		_shouldClose = 1;
	if(_ffmpegVideo)
		delete _ffmpegVideo;
	if(clust && clust->isServer())
		clust->abortJobs();

}

bool	Renderer::shouldClose(void) const
{
	return(_shouldClose);
}

int	Renderer::rendering(void) const
{
	return(_destPathIndex != 0 && !_testMode);
}
