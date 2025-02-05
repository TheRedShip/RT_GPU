/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:34:53 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/05 17:21:00 by tomoron          ###   ########.fr       */
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

void	Renderer::rawRead(std::ifstream &file, void *buf, size_t len)
{
	file.read((char *)buf, len);
	if(file.fail())
		throw std::runtime_error("syntax error in path file");
}

/* path file format (bytes):
 *	- output file name (terminated by a \0)
 *	- codec id
 *	- samples per image
 *	- fps
 *	- path nodes (until the end)
 */
void	Renderer::savePath(void)
{
	std::ofstream outputFile;
	const AVCodec *codec;
	(void)codec;

	codec = _codecList[_codecIndex];
	outputFile.open("output.path", std::ios::binary);
	if(!outputFile.is_open())
		std::cerr << "could open output.path for writing" << std::endl;
	outputFile.write(_outputFilename.c_str(), _outputFilename.length() + 1);
	outputFile.write((char *)&codec->id, sizeof(codec->id));
	outputFile.write((char *)&_samples, sizeof(_samples));
	outputFile.write((char *)&_fps, sizeof(_fps));
	for(std::vector<t_pathPoint>::iterator it = _path.begin(); it != _path.end(); it++)
		outputFile.write((char *)&(*it), sizeof(t_pathPoint));
	outputFile.close();
}


void	Renderer::loadPath(std::string filename)
{
	std::ifstream file;
	AVCodecID codecId;
	t_pathPoint pathPoint;
	std::vector<t_pathPoint>::iterator pos;
	char c;

	_outputFilename = "";
	_filenameBuffer[0] = 0;
	file.open(filename);
	if(!file.is_open())
		std::cerr << "failed to open " << filename  << std::endl;
	c = 1;
	while(c)
	{
		rawRead(file, &c, 1);
		if(c && c < 32 && c > 126)
			throw std::runtime_error("invalid char in filename");
		if(c)
			_outputFilename += c;
	}
	memcpy(_filenameBuffer, _outputFilename.c_str(), _outputFilename.length());
	_filenameBuffer[_outputFilename.length()] = 0;
	rawRead(file, &codecId, sizeof(codecId));
	updateAvailableCodecs(2, codecId);
	if(_codecList.size() == 0)
		throw std::runtime_error("codec not available");
	rawRead(file, &_samples, sizeof(_samples));
	rawRead(file, &_fps, sizeof(_fps));
	if(_samples < 1 || _fps < 1 || _samples >= 1000 || _fps >= 120)
		throw std::runtime_error("invalid value provided in fps or samples");
	while(file.peek() != EOF)
	{
		rawRead(file, &pathPoint, sizeof(t_pathPoint));
		if(pathPoint.time < .0f)
			throw std::runtime_error("invalid time provided in path");
		pos = _path.begin();
		while(pos != _path.end() && pos->time <= pathPoint.time)
			pos++;
		_path.insert(pos, pathPoint);
	}
	if(_path.size() < 2)
		throw std::runtime_error("not enough path points provided in the path");
	file.close();
}

void	Renderer::fillGoodCodecList(std::vector<AVCodecID> &lst)
{
	lst.push_back(AV_CODEC_ID_FFV1);
	lst.push_back(AV_CODEC_ID_H264);
	lst.push_back(AV_CODEC_ID_HUFFYUV);
	lst.push_back(AV_CODEC_ID_UTVIDEO);
	lst.push_back(AV_CODEC_ID_PRORES);
	lst.push_back(AV_CODEC_ID_V210);
}

/* modes :
 * 	0 : adds only supported codecs are showed
 * 	1 : adds all codecs
 * 	2 : adds only codec <id>
 */
void	Renderer::updateAvailableCodecs(int mode, AVCodecID id)
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
		if (mode == 1 || (mode == 2 && codec->id == id))
		{
			_codecList.push_back(codec);
			continue;
		}
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
	if(_path.size() < 2)
		throw std::runtime_error("render path doesn't have enough path points");
	if(_path[0].time != 0)
		throw std::runtime_error("render path does not start at 0, aborting");
	if(_path[_path.size() - 1].time - _path[0].time <= 0)
		throw std::runtime_error("render path is 0 seconds long, aborting");

	_codecOptions = 0;
	_destPathIndex = _path.size() - 1;
	_curPathIndex = 0;
	_frameCount = 0;
	_curSamples = 0;
	_curSplitStart = 0;
	_testMode = 0;
	_renderStartTime = glfwGetTime();
	_scene->getCamera()->setPosition(_path[0].pos);
	_scene->getCamera()->setDirection(_path[0].dir.x, _path[0].dir.y);
	_win->setFrameCount(_headless ? 0 : -1);
	avformat_alloc_output_context2(&_format, nullptr, nullptr, _outputFilename.c_str());
	
	_codec_context = avcodec_alloc_context3(_codecList[_codecIndex]);
	_codec_context->width = WIDTH;
	_codec_context->height = HEIGHT;
	_codec_context->time_base = {1, _fps};
	_codec_context->framerate = {_fps, 1};
	_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
	_codec_context->gop_size = 10;
	_codec_context->max_b_frames = 1;
	if(_codecList[_codecIndex]->id == AV_CODEC_ID_H264 || _codecList[_codecIndex]->id == AV_CODEC_ID_HEVC)
		av_dict_set(&_codecOptions, "crf", "0", 0);

	if (_format->oformat->flags & AVFMT_GLOBALHEADER)
		_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (avcodec_open2(_codec_context, _codecList[_codecIndex], &_codecOptions) < 0)
	{
		endRender();
		throw std::runtime_error("Failed to open codec");
	}
	
	_stream = avformat_new_stream(_format, _codecList[_codecIndex]);
	if (!_stream) 
	{
		endRender();
		throw std::runtime_error("Failed to create stream");
	}
	_stream->time_base = _codec_context->time_base;
	avcodec_parameters_from_context(_stream->codecpar, _codec_context);

	if (!(_format->flags & AVFMT_NOFILE))
	{
		if (avio_open(&_format->pb, _outputFilename.c_str(), AVIO_FLAG_WRITE) < 0)
		{
			endRender();
			throw std::runtime_error("couldn't open " + _outputFilename);
		}
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

	if(_codec_context)
	{
		avcodec_send_frame(_codec_context, 0);
		pkt = av_packet_alloc();
		while (avcodec_receive_packet(_codec_context, pkt) == 0) {
			pkt->stream_index = _stream->index;
			pkt->pts = av_rescale_q(pkt->pts, _codec_context->time_base, _stream->time_base);
			pkt->dts = av_rescale_q(pkt->dts, _codec_context->time_base, _stream->time_base);
			av_interleaved_write_frame(_format, pkt);
			av_packet_unref(pkt);
		}
	}
	av_packet_free(&pkt);

	if(_format)
		av_write_trailer(_format);
	if(_rgb_frame)
	    av_frame_free(&_rgb_frame);
	if(_yuv_frame)
	    av_frame_free(&_yuv_frame);
	if(_codec_context)
	    avcodec_free_context(&_codec_context);
	if(_format)
	    avio_close(_format->pb);
	if(_format)
	    avformat_free_context(_format);
	if(_codecOptions)
		av_dict_free(&_codecOptions);

	_format = 0;
	_rgb_frame = 0;
	_yuv_frame = 0;
	_codec_context = 0;
	_destPathIndex = 0;
	if(_headless)
		_shouldClose = 1;
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

void Renderer::update(Shader &shader)
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

glm::vec2 Renderer::bezierSphereInterpolate(glm::vec4 control, glm::vec2 from, glm::vec2 to, float time)
{
	glm::vec2 delta;
	glm::vec2 p1, p2;
	float t;

	p1 = glm::vec2(control.x, control.y);
	p2 = glm::vec2(control.z, control.w);
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
	bool			smallDistPrev;
	bool			smallDistNext;
	glm::vec4		bezierControl;

	from = _path[_curPathIndex];
	to = _path[_curPathIndex + 1];
	if(_curPathIndex)
		prev = _path[_curPathIndex - 1];
	else
		prev = from;
	if((size_t)_curPathIndex + 2 < _path.size())
		next = _path[_curPathIndex + 2];
	else
		next = to;

	cam = _scene->getCamera();
	pathTime = (to.time - from.time) * 60;
	normalTime = 1 - ((pathTime - timeFromStart) / pathTime);
	
	glm::vec3 points[4] = {prev.pos, from.pos, to.pos, next.pos};
	pos = hermiteInterpolate(points, normalTime);

	smallDistPrev = glm::distance((to.dir - from.dir) / glm::vec2(pathTime), (from.dir - prev.dir) / glm::vec2((from.time - prev.time) * 60)) < 40;
	smallDistNext = glm::distance((to.dir - from.dir) / glm::vec2(pathTime), (next.dir - to.dir) / glm::vec2((next.time - to.time) * 60)) < 40;
	bezierControl.x = 0.2f;
	bezierControl.y = !_curPathIndex || smallDistPrev ? .1f : .0f;
	bezierControl.z = 0.8f;
	bezierControl.w = (size_t)_curPathIndex + 2 >= _path.size() ||  smallDistNext ? .9f : 1.0f;

	dir = bezierSphereInterpolate(bezierControl, from.dir, to.dir, normalTime);
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

float Renderer::calcTime(void)
{
	float prevSpeed;
	float time;

	if(_path.size() > 1)
		prevSpeed = glm::distance(_path[_path.size() - 2].pos, _path[_path.size() - 1].pos) / (_path[_path.size() - 1].time - _path[_path.size() - 2].time);
	else
		prevSpeed = 0;

	if(_autoTime)
	{
		_tp = 0;
		if(_path.size() > 1)
			time = _path[_path.size() - 1].time + (glm::distance(_path[_path.size() - 1].pos, _scene->getCamera()->getPosition()) / prevSpeed);
		else
			time = (float)_path.size() / 60;
		if(std::isnan(time))
			time = _path[_path.size() - 1].time + (1.0f / 60);
		_min = time;
		_sec = (time - (int)time) * 60;
	}
	else if(_tp)
	{
		_autoTime = 0;
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
	time = calcTime();

	ImGui::Checkbox("guess time automatically", &_autoTime);
	ImGui::Checkbox("tp", &_tp);
	if(ImGui::Button("add step"))
		addPoint(time);


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
		ImGui::Separator();
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


bool	Renderer::shouldClose(void) const
{
	return(_shouldClose);
}

int	Renderer::rendering(void) const
{
	return(_destPathIndex != 0 && !_testMode);
}
