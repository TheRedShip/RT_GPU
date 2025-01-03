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
	object_parsers["sp"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Sphere(ss)); }
		catch (const std::exception &e) { throw; }
	};

	object_parsers["pl"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Plane(ss)); }
		catch (const std::exception &e) { throw; }
	};

	object_parsers["qu"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Quad(ss)); }
		catch (const std::exception &e) { throw; }
	};
}

void	SceneParser::parseMaterial(std::stringstream &line)
{
	float		r,g,b;
	float		emission;
	float		roughness;
	float		specular;
	Material	*mat;

	if (!(line >> r >> g >> b >> emission >> roughness >> specular))
		throw std::runtime_error("Material: Missing material properties");

	mat = new Material;

	mat->color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	mat->emission = emission;
	mat->roughness = roughness;
	mat->specular = specular;

	_scene->addMaterial(mat);
}

bool		SceneParser::parseLine(const std::string &line)
{
	if (line.empty() || line[0] == '#')
		return (true);
	
	std::stringstream	ss(line);
	std::string			identifier;
	
	ss >> identifier;

	try
	{
		auto it = object_parsers.find(identifier);
		if (it != object_parsers.end())
		{
			Object *obj = it->second(ss);
			(void) _scene->getMaterial(obj->getMaterialIndex()); //verify material
			
			_scene->addObject(obj);
		}

		if (identifier == "MAT")
			this->parseMaterial(ss);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (false);
	}

	return (true);
}
