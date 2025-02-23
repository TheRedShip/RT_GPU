/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   saveLoad.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 16:05:52 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/24 00:35:18 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

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
	Ffmpeg::updateAvailableCodecs(_codecList, _codecListStr, _outputFilename, _ignoreUnavailableCodec, (AVCodecID)0);
	_codecIndex = 0;
	if(_codecList.size() == 0)
		throw std::runtime_error("codec not available");
	rawRead(file, &_samples, sizeof(_samples));
	rawRead(file, &_fps, sizeof(_fps));
	if(_samples < 1 || _fps < 1 || _samples > 1000 || _fps > 120)
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
