/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:29:26 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/25 17:07:46 by tomoron          ###   ########.fr       */
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
	float		time;
}	t_pathPoint;

class Renderer
{
	public:
		Renderer(Scene *scene, Window *win);
		void	renderImgui(void);
		void	update(Shader &shader);

	private:
		void	addPoint(void);
		void	makeMovement(float timeFromStart, float curSplitTimeReset);
		void	initRender(std::string filename);
		void	addImageToRender(Shader &shader);
		void	endRender(void);

		int							_min;
		int							_sec;
		int							_samples;
		int							_testSamples;
		int							_fps;
		std::vector<t_pathPoint>	_path;
		Scene						*_scene;
		Window						*_win;

		int							_curPathIndex;
		int							_destPathIndex;
		double						_curSplitStart;
		int							_curSamples;
		int							_testMode;
		long int					_frameCount;

		AVFormatContext				*_format;
		AVCodecContext				*_codec_context;
		AVFrame						*_rgb_frame;
		AVFrame						*_yuv_frame;
		SwsContext					*_sws_context;
		AVStream					*_stream;

};

#endif
