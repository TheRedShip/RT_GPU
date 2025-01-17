/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ObjParser.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 15:00:49 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/17 14:53:58 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OBJPARSER_HPP
# define OBJPARSER_HPP

#include "RT.hpp"

class ObjParser
{
	public:
		ObjParser(std::string &filename);
		~ObjParser();
		void		parse(Scene &scene);

	private:
		glm::vec3	getVertex(std::stringstream &line);
		void 		addFace(std::stringstream &line, std::vector<glm::vec3> &vertices, int mat, Scene &scene);
		long int	checkVertexIndex(int index, size_t size);
		void		parseMtl(std::stringstream &line, std::map<std::string, int> &materials, Scene &scene);
		bool 		addTriangleFromPolygon(std::vector<glm::vec3> &vertices, Scene &scene, int mat, int inv);
		void		addTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, int mat, Scene &scene);
		int			pointInTriangle(glm::vec3 pts[3], std::vector<glm::vec3> vertices, size_t cur);

		std::ifstream			_file;
		std::vector<glm::vec3>	vertex;
};

#endif 
