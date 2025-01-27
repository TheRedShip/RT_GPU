/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/27 16:31:11 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

Renderer::Renderer(Scene *scene, Window *win)
{
	_scene = scene;
	_win = win;
	_min = 0;
	_sec = 0;
	_fps = 30;
	_samples = 1;
	_testSamples = 1;
	_curSamples = 0;
	_destPathIndex = 0;
	_frameCount = 0;

	_rgb_frame = 0;
	_yuv_frame = 0;
	_format = 0;
	_codec_context = 0;
}

void	Renderer::initRender(std::string filename)
{
	const AVCodec *codec;
	
	_destPathIndex = _path.size() - 1;
	_curPathIndex = 0;
	_frameCount = 0;
	_curSamples = 0;
	_curSplitStart = 0;
	_testMode = 0;
	_renderStartTime = glfwGetTime();
	_scene->getCamera()->setPosition(_path[0].pos);
	_scene->getCamera()->setDirection(_path[0].dir.x, _path[0].dir.y);
	_win->setFrameCount(-1);
	avformat_alloc_output_context2(&_format, nullptr, nullptr, filename.c_str());
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
		throw std::runtime_error("unable to find H264 audio/video codec, glhf");
	
	_codec_context = avcodec_alloc_context3(codec);
	_codec_context->width = WIDTH;
	_codec_context->height = HEIGHT;
	_codec_context->time_base = {1, _fps};
	_codec_context->framerate = {_fps, 1};
	_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
	_codec_context->gop_size = 10;
	_codec_context->max_b_frames = 1;

	if (_format->oformat->flags & AVFMT_GLOBALHEADER)
		_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (avcodec_open2(_codec_context, codec, nullptr) < 0)
		throw std::runtime_error("Failed to open codec");
	
	_stream = avformat_new_stream(_format, codec);
	if (!_stream) 
		throw std::runtime_error("Failed to create stream");
	_stream->time_base = _codec_context->time_base;
	avcodec_parameters_from_context(_stream->codecpar, _codec_context);

	if (!(_format->flags & AVFMT_NOFILE))
	{
		if (avio_open(&_format->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0)
			throw std::runtime_error("couldn't open " + filename);
	}
	(void)avformat_write_header(_format, nullptr);

    _rgb_frame = av_frame_alloc();
    _rgb_frame->format = AV_PIX_FMT_RGB24;
    _rgb_frame->width = WIDTH;
    _rgb_frame->height = HEIGHT;
    av_image_alloc(_rgb_frame->data, _rgb_frame->linesize, WIDTH, HEIGHT, AV_PIX_FMT_RGB24, 32);

    _yuv_frame = av_frame_alloc();
    _yuv_frame->format = _codec_context->pix_fmt;
    _yuv_frame->width = WIDTH;
    _yuv_frame->height = HEIGHT;
    av_image_alloc(_yuv_frame->data, _yuv_frame->linesize, WIDTH, HEIGHT, _codec_context->pix_fmt, 32);

	_sws_context = sws_getContext(
        WIDTH, HEIGHT, AV_PIX_FMT_RGB24,
        WIDTH, HEIGHT, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
}

void	Renderer::addImageToRender(Shader &shader)
{
	std::vector<float> image;
	AVPacket *pkt;
	long int videoFrameOffset;
	long int outputImageOffset;

	image = shader.getOutputImage();

	for (int x = 0; x < WIDTH; x++)
	{
		for(int y = 0; y < HEIGHT; y++)
		{
			videoFrameOffset = (y * _rgb_frame->linesize[0]) + (x * 3);
			outputImageOffset = (((HEIGHT - 1) - y) * (WIDTH * 4)) + (x * 4);
			glm::vec3 colors(image[outputImageOffset], image[outputImageOffset + 1], image[outputImageOffset + 2]);
		//	if(colors.x > 1 || colors.y > 1 || colors.z > 1)
		//		colors = glm::normalize(colors);
			colors.x = fmin(colors.x, 1);
			colors.y = fmin(colors.y, 1);
			colors.z = fmin(colors.z, 1);
			_rgb_frame->data[0][videoFrameOffset] = colors.x * 255;
			_rgb_frame->data[0][videoFrameOffset + 1] = colors.y * 255;
			_rgb_frame->data[0][videoFrameOffset + 2] = colors.z * 255;
		}
	}
	sws_scale(_sws_context, _rgb_frame->data, _rgb_frame->linesize, 0, HEIGHT, _yuv_frame->data, _yuv_frame->linesize);
	_yuv_frame->pts = _frameCount;

	if (avcodec_send_frame(_codec_context, _yuv_frame) == 0) {
		pkt = av_packet_alloc();
		while (avcodec_receive_packet(_codec_context, pkt) == 0) {
			pkt->stream_index = _stream->index;
        	pkt->pts = av_rescale_q(pkt->pts, _codec_context->time_base, _stream->time_base);
        	pkt->dts = av_rescale_q(pkt->dts, _codec_context->time_base, _stream->time_base);
			av_interleaved_write_frame(_format, pkt);
			av_packet_unref(pkt);
		}
		av_packet_free(&pkt);
	}
}

void	Renderer::endRender(void)
{
	AVPacket *pkt;

	avcodec_send_frame(_codec_context, 0);
	pkt = av_packet_alloc();
	while (avcodec_receive_packet(_codec_context, pkt) == 0) {
		pkt->stream_index = _stream->index;
		pkt->pts = av_rescale_q(pkt->pts, _codec_context->time_base, _stream->time_base);
		pkt->dts = av_rescale_q(pkt->dts, _codec_context->time_base, _stream->time_base);
		av_interleaved_write_frame(_format, pkt);
		av_packet_unref(pkt);
	}
	av_packet_free(&pkt);

	av_write_trailer(_format);
    av_frame_free(&_rgb_frame);
    av_frame_free(&_yuv_frame);
    avcodec_free_context(&_codec_context);
    avio_close(_format->pb);
    avformat_free_context(_format);

	_format = 0;
	_rgb_frame = 0;
	_yuv_frame = 0;
	_codec_context = 0;
}

void	Renderer::addPoint(void)
{
	t_pathPoint newPoint;
	Camera		*cam;
	std::vector<t_pathPoint>::iterator pos;

	cam = _scene->getCamera();
	newPoint.pos = cam->getPosition();	
	newPoint.dir = cam->getDirection();
	newPoint.time = _min + ((float)_sec / 60);
	pos = _path.begin();
	while(pos != _path.end() && pos->time <= newPoint.time)
		pos++;
	_path.insert(pos, newPoint);
}

void Renderer::update(Shader &shader)
{
	double			curTime;

	(void)shader;
	if(!_destPathIndex)
		return;

	_curSamples++;
	if((_testMode && _curSamples < _testSamples) || (!_testMode && _curSamples < _samples))
		return;

	if(_testMode)
		curTime = glfwGetTime();
	else
		curTime = (1 / (double)_fps) * (double)_frameCount;

	if(!_testMode)
	{
		addImageToRender(shader);
		_frameCount++;
	}
	makeMovement(curTime - _curSplitStart, curTime);
	_curSamples = 0;
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
		if(!_testMode)
			endRender();
	}
}

int	Renderer::rendering(void) const
{
	return(_destPathIndex != 0 && !_testMode);
}

void Renderer::imguiPathCreation(void)
{
	ImGui::SliderInt("test spi", &_testSamples, 1, 10);
	ImGui::SliderInt("render spi", &_samples, 1, 1000);
	ImGui::SliderInt("render fps", &_fps, 30, 120);
	if(_path.size() && ImGui::Button("try full path"))
	{
		_scene->getCamera()->setPosition(_path[0].pos);	
		_scene->getCamera()->setDirection(_path[0].dir.x, _path[0].dir.y);	
		_win->setFrameCount(-1);
		_curSplitStart = glfwGetTime();
		_curPathIndex = 0;
		_destPathIndex = _path.size() - 1;
		_testMode = 1;
	}
	if(_path.size() && ImGui::Button("start render"))
	{
		initRender("output.mp4");
	}
	ImGui::Separator();

	ImGui::SliderInt("minutes", &_min, 0, 2);
	ImGui::SliderInt("seconds", &_sec, 0, 60);
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
			_testMode = 1;
		}
		if(i > 1 && ImGui::Button(("match prev speed##" + std::to_string(i)).c_str()))
		{
			float speed = glm::distance(_path[i - 2].pos, _path[i - 1].pos) / (_path[i - 1].time - _path[i - 2].time);
			std::cout << "speed : " << speed << std::endl;
			std::cout << "dist : " << glm::distance(_path[i - 1].pos, _path[i].pos) << std::endl;
			_path[i].time = _path[i - 1].time + (glm::distance(_path[i - 1].pos, _path[i].pos) / speed);
		}
		ImGui::Separator();
	}
}

std::string	Renderer::floatToTime(float timef)
{
	std::string res;
	uint64_t time;
	uint64_t values[7];
	int firstValue;

	time = timef;
	values[0] = time / 3600 * 24 * 365;
	time = time % (3600 * 24 * 365);
	values[1] = time / 3600 * 24 * 30;
	time = time % (3600 * 24 * 30);
	values[2] = time / 3600 * 24 * 7;
	time = time % (3600 * 24 * 7);
	values[3] = time / 3600 * 24;
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
			res += std::to_string(values[0]);
			res += "Y";
		case 1:
			res += std::to_string(values[1]);
			res += "M";
		case 2:
			res += std::to_string(values[2]);
			res += "W";
		case 3:
			res += std::to_string(values[3]);
			res += "d";
		case 4:
			res += std::to_string(values[4]);
			res += "h";
		case 5:
			res += std::to_string(values[5]);
			res += "m";
		case 6:
			res += std::to_string(values[6]);
			res += "s";
	}
	return(res);
}

void Renderer::imguiRenderInfo(void)
{
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
	ImGui::Text("render in progress");
	ImGui::Text("samples per frame : %d", _samples);
	ImGui::Text("render fps : %d", _fps);
	ImGui::Text("total render time : %s", floatToTime((_path[_destPathIndex].time - _path[0].time) * 60).c_str());
	ImGui::Separator();
	ImGui::Text("Frames : %ld / %ld", _frameCount, totalFrames);
	ImGui::Text("Frames (with accumulation) : %ld / %ld", (_frameCount * _samples) + _curSamples, totalFrames * _samples);
	ImGui::Text("Render time : %dm%f", (int)renderTime, (renderTime - (int)renderTime) * 60);
	ImGui::Text("elapsed time : %s", floatToTime(timeElapsed).c_str());
	ImGui::Text("estimated time remaining : %s", floatToTime(timeEst).c_str());
	progress = ((float)_frameCount * _samples)  + _curSamples;
	progress /= (float)totalFrames * _samples;
	ImGui::ProgressBar(progress, ImVec2(0, 0));
	if(ImGui::Button("stop"))
	{
		_destPathIndex = 0;
		endRender();
	}
}

void Renderer::renderImgui(void)
{
	ImGui::Begin("Renderer");

	if(rendering())
		imguiRenderInfo();
	else
		imguiPathCreation();
	ImGui::End();
}
