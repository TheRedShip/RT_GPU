/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ffmpeg.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 23:41:18 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/24 00:41:00 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FFMPEG_HPP
# define FFMPEG_HPP

#include "RT.hpp"

class Ffmpeg
{
	public:
		Ffmpeg(std::string filename, int fps, const AVCodec *codec);
		~Ffmpeg();

		void		addImageToVideo(Scene &scene, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);

		static void	updateAvailableCodecs(std::vector<const AVCodec *> &codecList, std::vector<const char *> &codecListStr, std::string filename, int mode, AVCodecID id);
	
	private:
		void	convertAndAddToVid(void);
		void	endVideo(void);
		
		static void	fillGoodCodecList(std::vector<AVCodecID> &lst);

		int64_t			_pts;
		AVFormatContext	*_format;
		AVCodecContext	*_codec_context;
		AVFrame			*_rgb_frame;
		AVFrame			*_yuv_frame;
		SwsContext		*_sws_context;
		AVStream		*_stream;
		AVDictionary	*_codecOptions;
};

#endif
