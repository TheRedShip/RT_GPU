/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:37:37 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/13 17:39:22 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SCENEPARSER__HPP
# define RT_SCENEPARSER__HPP

# include "RT.hpp"

class SceneParser
{
	public:
		SceneParser(Scene *scene);

		bool		parseLine(const std::string &line);

	private:
		void		parseMaterial(std::stringstream &line);
		void		parseCamera(std::stringstream &line);
		void		parseObj(std::stringstream &line);
		glm::vec3	getVertex(std::stringstream &line);
		Triangle	*getFace(std::stringstream &line, std::vector<glm::vec3> &vertices);
		long int	getVertexIndex(std::stringstream &line, size_t size);

		Scene		*_scene;

		std::map<std::string, std::function<Object *(std::stringstream &)>> object_parsers;
};

#endif
