/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:29:26 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/04 21:56:58 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RENDERER_HPP
# define RENDERER_HPP

# include "RT.hpp"
extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
}

class Scene;
class Window;
class Shader;

typedef struct s_pathPoint
{
	glm::vec3	pos;
	glm::vec2	dir;
	double		time;
}	t_pathPoint;

class Renderer
{
	public:
		Renderer(Scene *scene, Window *win, Arguments &args);
		void	renderImgui(void);
		void	update(Shader &shader);
		int		rendering(void) const;
		bool	shouldClose(void) const;

	private:
		void	addPoint(float time);
		void	init(Scene *scene, Window *win);
		void	savePath(void);
		void	rawRead(std::ifstream &file, void *buf, size_t len);
		void	loadPath(std::string filename);
		void	makeMovement(float timeFromStart, float curSplitTimeReset);
		void	initRender();
		void	addImageToRender(Shader &shader);
		void	endRender(void);
		void	imguiPathCreation(void);
		void	imguiRenderInfo(void); 
		void	imguiRenderSettings(void);
		std::string	floatToTime(double timef);
		glm::vec2 bezierSphereInterpolate(glm::vec4 control, glm::vec2 from, glm::vec2 to, float time);
		void	updateAvailableCodecs(int mode, AVCodecID id);
		void	fillGoodCodecList(std::vector<AVCodecID> &lst);
		glm::vec3	hermiteInterpolate(glm::vec3 points[4], double alpha);

		int								_min;
		int								_sec;
		int								_samples;
		int								_testSamples;
		bool							_autoTime;
		int								_fps;
		char							_filenameBuffer[512];
		std::vector<t_pathPoint>		_path;
		std::string						_outputFilename;
		Scene							*_scene;
		Window							*_win;
		std::vector<const AVCodec *>	_codecList;
		std::vector<const char *>		_codecListStr;
		int								_codecIndex;
		bool							_renderSettings;
		bool							_ignoreUnavailableCodec;
		bool							_headless;
		bool							_tp;
		bool							_shouldClose;

		int								_curPathIndex;
		int								_destPathIndex;
		double							_curSplitStart;
		int								_curSamples;
		int								_testMode;
		long int						_frameCount;
		float							_renderStartTime;

		AVFormatContext					*_format;
		AVCodecContext					*_codec_context;
		AVFrame							*_rgb_frame;
		AVFrame							*_yuv_frame;
		SwsContext						*_sws_context;
		AVStream						*_stream;
		AVDictionary					*_codecOptions;

};

#endif
