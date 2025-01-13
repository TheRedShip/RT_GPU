/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:43:51 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/13 17:59:30 by tomoron          ###   ########.fr       */
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

	object_parsers["cy"] = [](std::stringstream &ss) -> Object *
	{
		try { return (new Cylinder(ss)); }
		catch (const std::exception &e) { throw; }
	};
}

void	SceneParser::parseMaterial(std::stringstream &line)
{
	float		r,g,b;
	float		emission;
	float		rough_refrac;
	float		metallic;
	std::string	type;

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
	
	mat->type = 0;
	if (type == "LAM")
		mat->type = 0;
	else if (type == "DIE")
		mat->type = 1;

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
	_scene->getCamera()->updateCameraVectors();
	
	_scene->getCamera()->setDOV(aperture, focus);
	_scene->getCamera()->setFov(fov);
	
	_scene->getCamera()->setBounce(bounce);

}

glm::vec3	SceneParser::getVertex(std::stringstream &line)
{
	glm::vec3 res;

	std::cout << line.str() << std::endl;
	if(!(line >> res.x >> res.y >> res.z))
		throw std::runtime_error("syntax error in obj file while parsing vertex");
	res.z *= -1;
	return(res);
}

long int	SceneParser::getVertexIndex(std::stringstream &line, size_t size)
{
	long int index;

	if(!(line >> index))
		throw std::runtime_error("syntax error in obj file while parsing face");
	if((size_t)index > size || index == 0 || (index < 0 && (size_t)(-index) > size))
		throw std::runtime_error("obj file error, invalid vertex index");	
	if(index < 0)
		index = size - index;
	return(index - 1);
}

Triangle	*SceneParser::getFace(std::stringstream &line, std::vector<glm::vec3> &vertices)
{
	glm::vec3 triangle[3];

	triangle[0] = vertices[getVertexIndex(line, vertices.size())];
	triangle[1] = vertices[getVertexIndex(line, vertices.size())];
	triangle[2] = vertices[getVertexIndex(line, vertices.size())];
	std::cout << triangle[0].x << " " << triangle[0].y << " " << triangle[0].z << std::endl;
	std::cout << triangle[1].x << " " << triangle[1].y << " " << triangle[1].z << std::endl;
	std::cout << triangle[2].x << " " << triangle[2].y << " " << triangle[2].z << std::endl;
	return(new Triangle(triangle[0], triangle[1], triangle[2], 0));		
}

void		SceneParser::parseObj(std::stringstream &objInfo)
{
	std::vector<glm::vec3>	vertices;
	std::string				filename;
	std::string				line;
	std::string				identifier;
	std::ifstream			file;

	objInfo >> filename;
	file.open(filename);
	if (!file.is_open())
		throw std::runtime_error("OBJ : could not open object file");
	while (getline(file, line))
	{
		if(line[0] == '#' || line.empty())
			continue;
		std::stringstream		lineStream(line);
		lineStream >> identifier;
		if(identifier == "v")
			vertices.push_back(getVertex(lineStream));
		else if (identifier == "f")
			_scene->addObject(getFace(lineStream, vertices));
	}
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
		else if (identifier == "OBJ")
			parseObj(ss);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (false);
	}

	return (true);
}
