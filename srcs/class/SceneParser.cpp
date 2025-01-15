/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneParser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/26 21:43:51 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/15 19:08:42 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SceneParser.hpp"

SceneParser::SceneParser(Scene *scene) : _scene(scene)
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

	if(!(line >> res.x >> res.y >> res.z))
		throw std::runtime_error("syntax error in obj file while parsing vertex");
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

Triangle	*SceneParser::getFace(std::stringstream &line, std::vector<glm::vec3> &vertices, int mat)
{
	glm::vec3 triangle[3];

	triangle[0] = vertices[getVertexIndex(line, vertices.size())];
	triangle[1] = vertices[getVertexIndex(line, vertices.size())];
	triangle[2] = vertices[getVertexIndex(line, vertices.size())];
	return (new Triangle(triangle[0], triangle[1], triangle[2], mat));		
}

void	SceneParser::parseMtl(std::stringstream &input_line, std::map<std::string, int> &materials)
{	
	std::string filename;
	std::ifstream file;
	std::string matName;
	std::string identifier;
	std::string line;
	Material *mat;

	input_line >> filename;
	file.open(filename);
	mat = 0;
	if(!file.is_open())
		throw std::runtime_error("OBJ : could not open material file");
	while(getline(file, line))
	{
		if(line[0] == '#' || !line[0])
			continue;
		std::stringstream lineStream(line);
		lineStream >> identifier;
		if(identifier == "newmtl")
		{
			if(mat)
			{
				_scene->addMaterial(mat);
				materials[matName] = _scene->getMaterialData().size() - 1;
			}
			lineStream >> matName;
			if(matName.empty())
				throw std::runtime_error("OBJ: syntax error in material file, missing material name");
			mat = new Material;
			bzero(mat, sizeof(Material));
			mat->metallic = 1.0f;
			continue;
		}
		if(!mat)
			throw std::runtime_error("OBJ: error in material file, material name not defined");
		if(identifier == "Kd")
		{
			if(!(lineStream >> mat->color.x >> mat->color.y >> mat->color.z))
				throw std::runtime_error("OBJ: syntax error while getting material color");
		}
		else if(identifier == "Ns")
		{
			if(!(lineStream >> mat->roughness) || mat->roughness > 1000 || mat->roughness < 0)
				throw std::runtime_error("OBJ: syntax error while getting material softness");
			mat->roughness /= 1000;
		}
		else if(identifier == "Ke")
		{
			float x, y, z;
			if(!(lineStream >> x >> y >> z))
				throw std::runtime_error("OBJ: syntax error while getting material emission");
			mat->emission = (x + y + z) / 3;
		}
		else if(identifier == "Ni")
		{
			if(!(lineStream >> mat->refraction))
				throw std::runtime_error("OBJ: syntax error while getting material refraction");
		}
		else
			std::cerr << "unsupported material setting : " << identifier << std::endl;
	}
	if(mat)
	{
		_scene->addMaterial(mat);
		materials[matName] = _scene->getMaterialData().size() - 1;
	}
}

void	SceneParser::parseObj(std::stringstream &objInfo)
{
	std::vector<glm::vec3>	vertices;
	std::string				filename;
	std::map<std::string, int> matNames;
	std::string				line;
	std::string				identifier;
	std::ifstream			file;
	int						curMat;

	objInfo >> filename;
	file.open(filename);
	curMat = 0;
	if (!file.is_open())
		throw std::runtime_error("OBJ : could not open object file");
	while (getline(file, line))
	{
		try{
			if(line[0] == '#' || line.empty())
				continue;
			std::stringstream		lineStream(line);
			identifier = "";
			lineStream >> identifier;
			if(identifier == "v")
				vertices.push_back(getVertex(lineStream));
			else if (identifier == "f")
				_scene->addObject(getFace(lineStream, vertices, curMat));
			else if (identifier == "mtllib")
				parseMtl(lineStream, matNames);
			else if (identifier == "usemtl")
			{
				lineStream >> identifier;
				if(matNames.find(identifier) == matNames.end())
					throw std::runtime_error("OBJ: invalid material name");
				curMat = matNames[identifier];
			}
		}catch (std::exception &e)
		{
			std::cerr << line << std::endl;
			throw;
		}
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
