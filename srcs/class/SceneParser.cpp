/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:43:51 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/26 21:43:51 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SceneParser.hpp"

SceneParser::SceneParser(Scene *scene) : _scene(scene)
{
	object_parsers["sp"] = [](std::stringstream& ss) -> Object*
	{
		try { return (new Sphere(ss)); }
		catch (const std::exception& e) { throw; }
	};
}

bool		SceneParser::parseLine(const std::string &line)
{
	if (line.empty() || line[0] == '#')
		return (true);
	
	std::stringstream	ss(line);
	std::string			identifier;
	
	ss >> identifier;

	auto it = object_parsers.find(identifier);
	if (it != object_parsers.end())
	{
		try {
			Object *obj = it->second(ss);
			_scene->addObject(obj);
		} catch (const std::exception& e) {
			std::cerr << "Error parsing sphere: " << e.what() << std::endl;
			return false;
		}
	}

	return (true);
}