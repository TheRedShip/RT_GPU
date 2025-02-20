/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ffmpeg.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 15:59:41 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 16:06:39 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					shaderDenoise(ShaderProgram &denoising_program, GPUDenoise &denoise, std::vector<GLuint> textures);

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


void		Renderer::addImageToRender(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	std::vector<float>	image(WIDTH * HEIGHT * 4);

	AVPacket *pkt;
	long int videoFrameOffset;
	long int outputImageOffset;
	

	if(_scene->getDenoise().enabled)
		shaderDenoise(denoisingProgram, _scene->getDenoise(), textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, image.data());
	glBindTexture(GL_TEXTURE_2D, 0);

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

void	Renderer::fillGoodCodecList(std::vector<AVCodecID> &lst)
{
	lst.push_back(AV_CODEC_ID_FFV1);
	lst.push_back(AV_CODEC_ID_H264);
	lst.push_back(AV_CODEC_ID_HUFFYUV);
	lst.push_back(AV_CODEC_ID_UTVIDEO);
	lst.push_back(AV_CODEC_ID_PRORES);
	lst.push_back(AV_CODEC_ID_V210);
}
