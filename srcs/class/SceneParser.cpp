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

	object_parsers["tr"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Triangle(ss)); }
		catch (const std::exception &e) { throw; }
	};

	object_parsers["cu"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Cube(ss)); }
		catch (const std::exception &e) { throw; }
	};
	
	object_parsers["po"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Portal(ss)); }
		catch (const std::exception &e) { throw; }
	};
}

void	SceneParser::parseMaterial(std::stringstream &line)
{
	float		r,g,b;
	float		emission;
	float		roughness;
	float		metallic;
	std::string	type;

	Material	*mat;

	if (!(line >> r >> g >> b >> emission >> roughness >> metallic))
		throw std::runtime_error("Material: Missing material properties");

	if (!(line >> type))
		type = "LAM";
	
	mat = new Material;

	mat->color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	mat->emission = emission;
	mat->roughness = roughness;
	mat->metallic = metallic;
	
	mat->type = 0;
	if (type == "LAM")
		mat->type = 0;
	else if (type == "DIE")
		mat->type = 1;

	_scene->addMaterial(mat);
}

void	SceneParser::parseCamera(std::stringstream &line)
{
	float		x,y,z;

	if (!(line >> x >> y >> z))
		throw std::runtime_error("Camera: Missing camera properties");

	_scene->getCamera()->setPosition(glm::vec3(x, y, z));
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
			
			if (obj->getType() == Object::Type::PORTAL)
				_scene->addObject(static_cast<Portal *>(obj)->createSupportQuad());
			
			_scene->addObject(obj);
		}

		if (identifier == "MAT")
			this->parseMaterial(ss);
		else if (identifier == "CAM")
			this->parseCamera(ss);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (false);
	}

	return (true);
}
