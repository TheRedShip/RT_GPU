/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/29 23:57:21 by tomoron          ###   ########.fr       */
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
	_outputFilename = "output.mp4";
	memcpy(_filenameBuffer, _outputFilename.c_str(), _outputFilename.length());
	_filenameBuffer[_outputFilename.length()] = 0;

	_rgb_frame = 0;
	_yuv_frame = 0;
	_format = 0;
	_codec_context = 0;
	updateAvailableCodecs();
}

void	Renderer::fillGoodCodecList(std::vector<AVCodecID> &lst)
{
	lst.push_back(AV_CODEC_ID_H264);
	lst.push_back(AV_CODEC_ID_MPEG4);
	lst.push_back(AV_CODEC_ID_H263);
	lst.push_back(AV_CODEC_ID_MPEG2VIDEO);
}

void	Renderer::updateAvailableCodecs(void)
{
	const AVCodec *codec;
	AVCodecContext *ctx;
	const AVOutputFormat *muxer;
	const char *format;
	std::vector<AVCodecID>	goodCodecList;

	fillGoodCodecList(goodCodecList);
	_codecList.clear();
	_codecListStr.clear();
	_codecIndex = 0;
	av_log_set_level(AV_LOG_QUIET);
	format = _outputFilename.c_str();
	if(_outputFilename.find(".") != std::string::npos)
		format += _outputFilename.find(".") + 1;

	muxer = av_guess_format(format, 0, 0);
	for(std::vector<AVCodecID>::iterator it = goodCodecList.begin(); it != goodCodecList.end(); it++)
	{
		codec = avcodec_find_encoder(*it);
		if(!codec)
			continue;
		ctx = avcodec_alloc_context3(codec);
		if(ctx)
		{
			if (avformat_query_codec(muxer, codec->id, FF_COMPLIANCE_STRICT) > 0)
			{
				ctx->width = WIDTH;
				ctx->height = HEIGHT;
				ctx->time_base = {1, _fps};
				ctx->framerate = {_fps, 1};
				ctx->pix_fmt = AV_PIX_FMT_YUV420P;
				ctx->gop_size = 10;
				ctx->max_b_frames = 1;
				if (avcodec_open2(ctx, codec, NULL) == 0)
					_codecList.push_back(codec);
	        }
			avcodec_free_context(&ctx);
		}
	}
	for(auto it = _codecList.begin(); it != _codecList.end(); it++)
	{
		_codecListStr.push_back((*it)->name);
	}
}

void	Renderer::initRender(void)
{
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
	avformat_alloc_output_context2(&_format, nullptr, nullptr, _outputFilename.c_str());
	
	_codec_context = avcodec_alloc_context3(_codecList[_codecIndex]);
	_codec_context->width = WIDTH;
	_codec_context->height = HEIGHT;
	_codec_context->time_base = {1, _fps};
	_codec_context->framerate = {_fps, 1};
	_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
	_codec_context->gop_size = 10;
	_codec_context->max_b_frames = 1;

	if (_format->oformat->flags & AVFMT_GLOBALHEADER)
		_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (avcodec_open2(_codec_context, _codecList[_codecIndex], nullptr) < 0)
		throw std::runtime_error("Failed to open codec");
	
	_stream = avformat_new_stream(_format, _codecList[_codecIndex]);
	if (!_stream) 
		throw std::runtime_error("Failed to create stream");
	_stream->time_base = _codec_context->time_base;
	avcodec_parameters_from_context(_stream->codecpar, _codec_context);

	if (!(_format->flags & AVFMT_NOFILE))
	{
		if (avio_open(&_format->pb, _outputFilename.c_str(), AVIO_FLAG_WRITE) < 0)
			throw std::runtime_error("couldn't open " + _outputFilename);
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

glm::vec3 Renderer::hermiteInterpolate(glm::vec3 points[4], double alpha)
{
	double tension;
	double bias;
	glm::vec3 tang[2];
	double alphaSqr[2];
	glm::vec3 coef[4];
	
	tension = 0;
	bias = 0;

	alphaSqr[0] = alpha * alpha;
	alphaSqr[1] = alphaSqr[0] * alpha;

	tang[0]  = (points[1] - points[0]) * glm::vec3(1 + bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[0] += (points[2] - points[1]) * glm::vec3(1 - bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[1]  = (points[2] - points[1]) * glm::vec3(1 + bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[1] += (points[3] - points[2]) * glm::vec3(1 - bias) * glm::vec3(1 - tension) / glm::vec3(2);

	coef[0] = glm::vec3(2 * alphaSqr[1] - 3 * alphaSqr[0] + 1);
	coef[1] = glm::vec3(alphaSqr[1] - 2 * alphaSqr[0] + alpha);
	coef[2] = glm::vec3(alphaSqr[1] -   alphaSqr[0]);
	coef[3] = glm::vec3(-2 * alphaSqr[1] + 3 * alphaSqr[0]);
	
	return(coef[0] * points[1] + coef[1] * tang[0] + coef[2] * tang[1] + coef[3] * points[2]);
}

glm::quat eulerToQuaternion(float pitch, float yaw)
{
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
    glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
    
    glm::quat result = qYaw* qPitch;
    return(result);
}

//glm::vec2 Renderer::sphereInterpolate(glm::vec2 from, glm::vec2 to, float time) // gud but bad
//{
//	glm::vec3	eulerRes;
//	glm::quat	qFrom;
//	glm::quat	qTo;
//	glm::quat	res;
//	float		angle;
//	float		dot;
//
//	qFrom = glm::normalize(eulerToQuaternion(from.y, from.x));
//	qTo = glm::normalize(eulerToQuaternion(to.y, to.x));
//	
//	dot = glm::dot(qFrom, qTo);
//	if(dot < 0)
//		to = -to;
//	angle = 2 * glm::acos(dot);
//	res = (glm::sin((1 - time) * angle / glm::sin(angle)) * qFrom) + ((glm::sin(time * angle) / glm::sin(angle)) * qTo);
//	eulerRes = glm::degrees(glm::eulerAngles(res));
//	return(glm::vec2(eulerRes.y, eulerRes.x));
//}


glm::vec2 Renderer::sphereInterpolate(glm::vec2 from, glm::vec2 to, float time)
{
	glm::vec2 delta;
	glm::vec2 p1, p2;
	float t;

	p1 = glm::vec2(0.20, 0);
	p2 = glm::vec2(0.80, 1);
	t = time;
    for(int i = 0; i < 5; i++) {
        float currentX = 3.0f * ((1 - t) * (1 - t)) * t * p1.x + 3.0f * (1 - t) * (t * t) * p2.x + (t * t * t);
        
        if(abs(currentX - time) < 0.00001f) {
            break;
        }
        
        glm::vec2 derivative = glm::vec2(
	        3.0f * (1 - t) * (1 - t) * p1.x +
	        6.0f * (1.0f - t) * t * (p2.x - p1.x) +
	        3.0f * t * t * (1.0f - p2.x),
	        
	        3.0f * (1 - t) * (1 - t) * p1.y +
	        6.0f * (1.0f - t) * t * (p2.y - p1.y) +
	        3.0f * t * t * (1.0f - p2.y)
	    );
        
        if(abs(derivative.x) > 0.00001f){
            t = t - (currentX - time) / derivative.x;
            t = glm::clamp(t, 0.0f, 1.0f);
        }
    }
	t = 3.0f * ((1 - t) * (1 - t)) * t * p1.y + 3.0f * (1 - t) * (t * t) * p2.y + (t * t * t);

	delta = to - from;
	return(from + glm::vec2(delta.x * t, delta.y * t));
}


void Renderer::makeMovement(float timeFromStart, float curSplitTimeReset)
{
	t_pathPoint		from;
	t_pathPoint		to;
	t_pathPoint		prev;
	t_pathPoint		next;
	float			pathTime;
	Camera			*cam;
	glm::vec3		pos;
	glm::vec2		dir;	
	float			normalTime;

	from = _path[_curPathIndex];
	to = _path[_curPathIndex + 1];
	if(_curPathIndex)
		prev = _path[_curPathIndex - 1];
	else
		prev = from;
	if((size_t)_curPathIndex + 3 == _path.size())
		next = _path[_curPathIndex + 2];
	else
		next = to;
		

	cam = _scene->getCamera();
	pathTime = (to.time - from.time) * 60;
	normalTime = 1 - ((pathTime - timeFromStart) / pathTime);
	
	pos = hermiteInterpolate((glm::vec3 [4]){prev.pos, from.pos, to.pos, next.pos}, normalTime);
	dir = sphereInterpolate(from.dir, to.dir, normalTime);
	if(std::isnan(dir.x) || std::isnan(dir.y))
		dir = from.dir;
	if(timeFromStart >= pathTime)
	{
		pos = to.pos;
		dir = to.dir;
		_curSplitStart = curSplitTimeReset;
		_curPathIndex++;
	}
	cam->setPosition(pos);
	cam->setDirection(dir.x, dir.y);
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
	
	ImGui::Combo("codec", &_codecIndex, _codecListStr.data(), _codecListStr.size());
	if(ImGui::InputText("file name", _filenameBuffer, 512))
	{
		_outputFilename = _filenameBuffer;
		updateAvailableCodecs();
	}
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
	if(_path.size() > 1 && _codecList.size() && ImGui::Button("start render"))
		initRender();

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
