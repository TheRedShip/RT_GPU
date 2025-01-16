/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ObjParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 15:00:33 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/16 22:00:46 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

ObjParser::ObjParser(std::string &filename)
{
	_file.open(filename);
	if(!_file.is_open())
		throw std::runtime_error("OBJ : could not open object file");
}

ObjParser::~ObjParser()
{
	_file.close();
}

glm::vec3	ObjParser::getVertex(std::stringstream &line)
{
	glm::vec3 res;

	if(!(line >> res.x >> res.y >> res.z))
		throw std::runtime_error("syntax error in obj file while parsing vertex");
	return(res);
}

long int	ObjParser::checkVertexIndex(int index, size_t size)
{
	if((size_t)index > size || index == 0 || (index < 0 && (size_t)(-index) > size))
		throw std::runtime_error("obj file error, invalid vertex index");	
	if(index < 0)
		index = size - index;
	return(index - 1);
}

//void ObjParser::isEar(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, std::vector<glm::vec3> &vertices)
//{
//	
//}

void ObjParser::addTriangleFromPolygon(std::vector<glm::vec3> &vertices, Scene &scene)
{
	glm::vec3 normal;
	glm::vec3 v1, v2 ,v3, v4;

	(void)scene;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		v1 = vertices[(i - 1) % vertices.size()];
		v2 = vertices[i];
		v3 = vertices[(i + 1) % vertices.size()];
		v4 = vertices[(i + 2) % vertices.size()];

		normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
		// std::cout << glm::to_string(normal) << std::endl;

		// float dot = glm::dot(normal, v4 - v1);
		// std::cout << dot << std::endl;
		//if(isEar(v1, v2 ,v3, vertices))
		//{
		//	vertices.erase(vertices.begin() + i);
		//	scene.addObject(new Triangle(v1, v2 ,v3));
		//}
	}
	// std::cout << std::endl;
	// std::cout << std::endl;
	// std::cout << std::endl;
	// std::cout << std::endl;
}

void ObjParser::addFace(std::stringstream &line, std::vector<glm::vec3> &vertices, int mat,  Scene &scene)
{
	int vert_index;
	std::vector<glm::vec3> face_vertices;

	(void)mat;
	(void)scene;
	while((line >> vert_index))
		face_vertices.push_back(vertices[checkVertexIndex(vert_index, vertices.size())]);
	if(face_vertices.size() < 3)
		throw std::runtime_error("OBJ : face does not have enough vertices");

	// for(size_t i = 0; i < face_vertices.size(); i++)
	// 	std::cout << face_vertices[i].x << " " << face_vertices[i].y << " " << face_vertices[i].z << " | ";
	// std::cout << std::endl;

//	while(face_vertices.size() > 3)
//		addTriangleFromPolygon(face_vertices, scene);
	if(!line.eof())
		throw std::runtime_error("OBJ: an error occured while parsing face");
	addTriangle(face_vertices[0], face_vertices[1], face_vertices[2],mat, scene);
}

void	ObjParser::addTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, int mat, Scene &scene)
{
	scene.addObject(new Triangle(v1, v2, v3, mat));
}

void	ObjParser::parseMtl(std::stringstream &input_line, std::map<std::string, int> &materials, Scene &scene)
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
		if (line.empty() || line[0] == '#')
			continue;
		std::stringstream lineStream(line);
		lineStream >> identifier;
		if(identifier == "newmtl")
		{
			if(mat)
			{
				scene.addMaterial(mat);
				materials[matName] = scene.getMaterialData().size() - 1;
			}
			lineStream >> matName;
			if(matName.empty())
				throw std::runtime_error("OBJ: syntax error in material file, missing material name");
			mat = new Material;
			memset(mat, 0, sizeof(Material));
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
		scene.addMaterial(mat);
		materials[matName] = scene.getMaterialData().size() - 1;
	}
	file.close();
}

void	ObjParser::parse(Scene &scene)
{
	std::vector<glm::vec3>	vertices;
	std::string				filename;
	std::map<std::string, int> matNames;
	std::string				line;
	std::string				identifier;
	std::ifstream			file;
	int						curMat;

	curMat = 0;
	while (getline(_file, line))
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
				addFace(lineStream, vertices, curMat, scene);
			else if (identifier == "mtllib")
				parseMtl(lineStream, matNames, scene);
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

