/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:43:51 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/04 01:21:11 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SceneParser.hpp"

SceneParser::SceneParser(Scene *scene, std::string filename) : _scene(scene), _filename(filename)
{
	object_parsers["sp"] = [](std::stringstream &ss) -> Object * { return (new Sphere(ss)); };
	object_parsers["pl"] = [](std::stringstream &ss) -> Object * { return (new Plane(ss)); };
	object_parsers["qu"] = [](std::stringstream &ss) -> Object * { return (new Quad(ss)); };
	object_parsers["tr"] = [](std::stringstream &ss) -> Object * { return (new Triangle(ss)); };
	object_parsers["cu"] = [](std::stringstream &ss) -> Object * { return (new Cube(ss)); };
	object_parsers["po"] = [](std::stringstream &ss) -> Object * { return (new Portal(ss)); };
	object_parsers["cy"] = [](std::stringstream &ss) -> Object * { return (new Cylinder(ss)); };
}

void	SceneParser::parseMaterial(std::stringstream &line)
{
	float		r,g,b;
	float		emission;
	float		rough_refrac;
	float		metallic;
	std::string	type;
	int			texture_index;

	Material	*mat;

	if (!(line >> r >> g >> b >> emission >> rough_refrac >> metallic))
		throw std::runtime_error("Material: Missing material properties");

	if (!(line >> type))
		type = "LAM";
		
	mat = new Material;
	mat->color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	mat->emission = emission;
	mat->roughness = rough_refrac;
	mat->metallic = metallic;
	mat->refraction = rough_refrac;
	
	mat->type = -1;
	if (type == "LAM")
		mat->type = 0;
	else if (type == "DIE")
		mat->type = 1;
	else if (type == "TRN")
		mat->type = 2;
	
	texture_index = -1;
	if (mat->type != -1)
		line >> texture_index;
	else
		mat->type = 0;

	mat->texture_index = texture_index;
	mat->emission_texture_index = -1;
	_scene->addMaterial(mat);
}

void	SceneParser::parseCamera(std::stringstream &line)
{
	float	x,y,z;
	float	yaw, pitch;
	float	aperture, focus, fov;
	int		bounce;
	
	if (!(line >> x >> y >> z))
		throw std::runtime_error("Camera: Missing camera properties");

	if (!(line >> yaw >> pitch))
	{
		yaw = 0;
		pitch = -90;
	}

	if (!(line >> aperture >> focus >> fov))
	{
		aperture = 0.0;
		focus = 1.0;
		fov = 90.0f;
	}

	if (!(line >> bounce))
		bounce = 5;

	_scene->getCamera()->setPosition(glm::vec3(x, y, z));
	_scene->getCamera()->setDirection(yaw, pitch);
	
	_scene->getCamera()->setDOV(aperture, focus);
	_scene->getCamera()->setFov(fov);
	
	_scene->getCamera()->setBounce(bounce);

}

void		SceneParser::parseObj(std::stringstream &line)
{
	std::string	name;
	float x = 0.;
	float y = 0.;
	float z = 0.;

	float scale = 1.;
	
	float xtransform = 0.;
	float ytransform = 0.;
	float ztransform = 0.;
	
	int mat = 0;

	line >> name;
	line >> x >> y >> z;
	line >> scale;
	line >> xtransform >> ytransform >> ztransform;
	line >> mat;

	glm::mat4 transform = glm::eulerAngleXYZ(glm::radians(xtransform), glm::radians(ytransform), glm::radians(ztransform));

	ObjParser obj(name, _filename, mat);
	obj.parse(*_scene, glm::vec3(x, y, z), (1.0 / scale), transform);
}

void		SceneParser::parseTexture(std::stringstream &line)
{
	std::string	path;
	
	if (!(line >> path))
		throw std::runtime_error("Texture: Missing texture's path");

	_scene->addTexture(path);
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
			
			GPUMaterial mat = _scene->getMaterial(obj->getMaterialIndex()); //verify material
			
			if (obj->getType() == Object::Type::PORTAL)
				_scene->addObject(static_cast<Portal *>(obj)->createSupportQuad());
			
			_scene->addObject(obj);

			if (mat.emission > 0.0)
				_scene->updateLightAndObjects(obj->getMaterialIndex());
		}

		if (identifier == "MAT")
			this->parseMaterial(ss);
		else if (identifier == "CAM")
			this->parseCamera(ss);
		else if (identifier == "OBJ")
			this->parseObj(ss);
		else if (identifier == "TEX")
			this->parseTexture(ss);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (false);
	}

	return (true);
}
