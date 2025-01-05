/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:37:37 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/26 21:37:37 by TheRed           ###   ########.fr       */
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

		Scene		*_scene;

		std::map<std::string, std::function<Object *(std::stringstream &)>> object_parsers;
};

#endif