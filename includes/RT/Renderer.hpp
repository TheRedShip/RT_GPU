/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:29:26 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/15 22:46:36 by tomoron          ###   ########.fr       */
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
class ShaderProgram;

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

		void		update(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		void		renderImgui(void);

		int			rendering(void) const;
		bool		shouldClose(void) const;

	private:
		void		init(Scene *scene, Window *win);

		void		showRenderInfo(int isImgui); 
		std::string	floatToTime(double timef);
		float		calcTime(void);

		void		addPoint(float time);
		void		imguiPathCreation(void);
		void		imguiRenderSettings(void);

		void		rawRead(std::ifstream &file, void *buf, size_t len);
		void		savePath(void);
		void		loadPath(std::string filename);

		void		makeMovement(float timeFromStart, float curSplitTimeReset);
		glm::vec2	bezierSphereInterpolate(glm::vec4 control, glm::vec2 from, glm::vec2 to, float time);
		glm::vec3	hermiteInterpolate(glm::vec3 points[4], double alpha);

		void		initRender();
		void		fillGoodCodecList(std::vector<AVCodecID> &lst);
		void		updateAvailableCodecs(int mode, AVCodecID id);
		void		addImageToRender(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		void		endRender(void);


		Scene							*_scene;
		Window							*_win;
		bool							_shouldClose;
		bool							_headless;
		std::string						_outputFilename;

		std::vector<t_pathPoint>		_path;
		int								_curPathIndex;
		int								_destPathIndex;
		double							_curSplitStart;
		int								_curSamples;
		int								_testMode;
		long int						_frameCount;
		float							_renderStartTime;


		int								_samples;
		int								_testSamples;
		int								_min;
		int								_sec;
		int								_fps;
		char							_filenameBuffer[512];
		bool							_ignoreUnavailableCodec;
		bool							_tp;
		bool							_autoTime;
		std::vector<const char *>		_codecListStr;
		int								_codecIndex;

		bool							_renderSettings;


		AVFormatContext					*_format;
		AVCodecContext					*_codec_context;
		AVFrame							*_rgb_frame;
		AVFrame							*_yuv_frame;
		SwsContext						*_sws_context;
		AVStream						*_stream;
		AVDictionary					*_codecOptions;
		std::vector<const AVCodec *>	_codecList;
};

#endif
