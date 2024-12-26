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

#ifndef RT_SceneParser__HPP
# define RT_SceneParser__HPP

# include "RT.hpp"

class SceneParser
{
	public:
		SceneParser(Scene *scene);

		bool		parseLine(const std::string &line);

	private:
		Scene *_scene;

		std::map<std::string, std::function<Object *(std::stringstream &)>> object_parsers;
};

#endif