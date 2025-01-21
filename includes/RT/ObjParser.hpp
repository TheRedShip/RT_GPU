/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ObjParser.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 15:00:49 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/21 16:01:21 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OBJPARSER_HPP
# define OBJPARSER_HPP

#include "RT.hpp"

class ObjParser
{
	public:
		ObjParser(std::string &filename, std::string &scene_filename);
		~ObjParser();

		void	parse(Scene &scene, glm::vec3 offset, float scale, glm::mat4 transform);
	
	private:
		glm::vec3					getVertex(std::stringstream &line);
		glm::vec2					getUV(std::stringstream &line);
		void 						addFace(std::stringstream &line);
		long int					checkVertexIndex(int index, size_t size);
		void						parseMtl(std::stringstream &line, Scene &scene);
		bool 						addTriangleFromPolygon(std::vector<glm::vec3> &vertices, int inv);
		void						addTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
		std::string					getFilePath(std::string &file);
		int							pointInTriangle(glm::vec3 pts[3], std::vector<glm::vec3> vertices, size_t cur);
		std::vector<std::string>	objSplit(std::string str, std::string delim);
		void						getFaceVertices(std::vector<glm::vec3> &faceVertices, std::stringstream &line);

		std::string					_filename;
		std::ifstream				_file;
		std::vector<glm::vec3>		_vertices;
		std::vector<glm::vec2>		_textureVertices;
		int							_mat;
		std::map<std::string, int>	_matNames;

		std::vector<Triangle>		_triangles;
};

#endif 
