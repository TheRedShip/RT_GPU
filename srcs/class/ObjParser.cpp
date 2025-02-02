/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ObjParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 15:00:33 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/31 19:53:44 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

ObjParser::ObjParser(std::string &filename, std::string &scene_filename, int mat)
{
	_mat = mat;
	_filename = getFilePath(scene_filename) + filename;
	std::cout << _filename << std::endl;
	_file.open(_filename);

	if(!_file.is_open())
		throw std::runtime_error("OBJ : could not open object file");
}

ObjParser::~ObjParser()
{
	_file.close();
}

std::string ObjParser::getFilePath(std::string &file)
{
	int index;

	if(file.find("/") == std::string::npos)
		return("");
	index = file.length() - 1;
	while(index && file[index] != '/')
		index--;
	return(file.substr(0, index + 1));
}

glm::vec3	ObjParser::getVertex(std::stringstream &line)
{
	glm::vec3 res;

	if(!(line >> res.x >> res.y >> res.z))
		throw std::runtime_error("syntax error in obj file while parsing vertex");
	return(res);
}

glm::vec2	ObjParser::getUV(std::stringstream &line)
{
	glm::vec2 res;

	if(!(line >> res.x) || (!(line >> res.y) && !line.eof()))
		throw std::runtime_error("syntax error in obj file while parsing texture vertex");
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

int ObjParser::pointInTriangle(glm::vec3 pts[3], std::vector<glm::vec3> vertices, size_t cur)
{
	glm::vec3 v0, v1, v2;
	float d00, d01, d11, d20, d21;
	float den;
	float u, v;

	for(size_t i = 0; i < vertices.size(); i++)
	{
		if(i == ((cur - 1) % vertices.size())  || i == cur || i == ((cur + 1) % vertices.size()))
				continue;
		v0 = pts[2] - pts[0];
		v1 = pts[1] - pts[0];
		v2 = vertices[i] - pts[0];	
		
		d00 = glm::dot(v0, v0);
		d01 = glm::dot(v0, v1);
		d11 = glm::dot(v1, v1);
		d20 = glm::dot(v2, v0);
		d21 = glm::dot(v2, v1);
		den = glm::dot(d00, d11) - glm::dot(d01, d01);

		u = (glm::dot(d11, d20) - glm::dot(d01, d21)) / den;
		v = (glm::dot(d00, d21) - glm::dot(d01, d20)) / den;
		if(u >= 0 && v >= 0 && (u + v) <= 1)
			return(1);
	}
	return(0);
}

bool ObjParser::addTriangleFromPolygon(std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &textureVertices, int inv)
{
	glm::vec3 v1, v2, v3;
	glm::vec2 vt1, vt2, vt3;

	float dot;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		if(!i)
		{
			v1 = vertices.back();
			vt1 = textureVertices.back();
		}
		else
		{
			v1 = vertices[i - 1];
			vt1 = textureVertices[i - 1];
		}
		v2 = vertices[i];
		v3 = vertices[(i + 1) % vertices.size()];

		vt2 = textureVertices[i];
		vt3 = textureVertices[(i + 1) % textureVertices.size()];

		if (inv)
			dot = glm::cross(v2 - v1, v2 - v3).z;
		else
			dot = glm::cross(v2 - v3, v2 - v1).z;
		if(dot <= 0)
			continue;
		glm::vec3 triangleVertices[3] = {v1, v2, v3};
		if(pointInTriangle(triangleVertices, vertices, i))
			continue;
		vertices.erase(vertices.begin() + i);
		textureVertices.erase(textureVertices.begin() + i);


		std::vector<glm::vec2> texture;
		texture.push_back(vt1);
		texture.push_back(vt2);
		texture.push_back(vt3);

		addTriangle(v1, v2, v3, texture);
		return(1);
	}
	return(0);
}

std::vector<std::string> ObjParser::objSplit(std::string str, std::string delim)
{
	std::vector<std::string> res;
	
	while(str.find(delim) != std::string::npos)
	{
		res.push_back(str.substr(0, str.find(delim)));
		str.erase(0, str.find(delim) + 1);
	}
	res.push_back(str);
	return(res);
}

void ObjParser::getFaceVertices(std::vector<glm::vec3> &faceVertices, std::vector<glm::vec2> &textureVertices, std::stringstream &line)
{
	std::string el;
	std::vector<std::string> sp;

	while(line >> el)
	{
		sp = objSplit(el, "/");
		if(sp.size() > 3)
			std::runtime_error("OBJ : too many values in an element of a face");
		if(sp.size() == 0)
			std::runtime_error("OBJ : wtf ?");
		
		if (sp.size() > 1 && sp[1].length())
			textureVertices.push_back(_textureVertices[checkVertexIndex(std::stoi(sp[1]), _textureVertices.size())]);
		faceVertices.push_back(_vertices[checkVertexIndex(std::stoi(sp[0]), _vertices.size())]);
	}
}

void ObjParser::addFace(std::stringstream &line)
{
	std::vector<glm::vec3> faceVertices;
	std::vector<glm::vec2> textureVertices;
	
	getFaceVertices(faceVertices, textureVertices, line);
	if(!line.eof())
		throw std::runtime_error("OBJ : an error occured while paring face");
	if(faceVertices.size() < 3)
		throw std::runtime_error("OBJ : face does not have enough vertices");

	while(faceVertices.size() > 3)
		if (!addTriangleFromPolygon(faceVertices, textureVertices, 0))
			if(!addTriangleFromPolygon(faceVertices, textureVertices, 1))
				return ;

	if(!line.eof())
		throw std::runtime_error("OBJ: an error occured while parsing face");
	addTriangle(faceVertices[0], faceVertices[1], faceVertices[2], textureVertices);
}

void	ObjParser::addTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, std::vector<glm::vec2> texture_vertices)
{
	glm::vec2 vt1 = glm::vec2(0.);
	glm::vec2 vt2 = glm::vec2(0.);
	glm::vec2 vt3 = glm::vec2(0.);

	if (texture_vertices.size() == 3)
	{
		vt1 = texture_vertices[0];
		vt2 = texture_vertices[1];
		vt3 = texture_vertices[2];
	}

	_triangles.push_back(Triangle(v1, v2, v3, vt1, vt2, vt3, _mat));
}

void	ObjParser::parseMtl(std::stringstream &input_line, Scene &scene)
{	
	std::string filename;
	std::ifstream file;
	std::string matName;
	std::string identifier;
	std::string line;
	Material *mat;

	input_line >> filename;
	filename = getFilePath(_filename) + filename;
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
				_matNames[matName] = scene.getMaterialData().size() - 1;
			}
			lineStream >> matName;
			if(matName.empty())
				throw std::runtime_error("OBJ: syntax error in material file, missing material name");
			mat = new Material;
			memset(mat, 0, sizeof(Material));
			mat->texture_index = -1;
			mat->emission_texture_index = -1;
			mat->refraction = 1.0f;
			mat->roughness = 1.0f;
			mat->metallic = 1.0f;
			mat->color = glm::vec3(1.0f, 1.0f, 1.0f);
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
		else if (identifier == "Pr")
		{
			if (!(lineStream >> mat->roughness) || mat->roughness > 1 || mat->roughness < 0)
				throw std::runtime_error("OBJ: syntax error while getting material softness");
			
			mat->roughness = 1 - mat->roughness;
		}
		else if (identifier == "Pm")
		{
			if (!(lineStream >> mat->metallic) || mat->metallic > 1 || mat->metallic < 0)
				throw std::runtime_error("OBJ: syntax error while getting material metallic");
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
		else if(identifier == "Tr")
		{
			float prob;

			if(!(lineStream >> prob)) 
				throw std::runtime_error("OBJ : syntax error while getting material transparency");
			if(prob == 0)
				continue;
			mat->metallic = 1 - prob;
			mat->type = 2;
		}
		else if(identifier == "d")
		{
			float prob;

			if(!(lineStream >> prob)) 
				throw std::runtime_error("OBJ : syntax error while getting material transparency");
			if(prob == 1)
				continue;
			mat->metallic = prob;
			mat->type = 2;
		}
		else if (identifier == "map_Kd")
		{
			std::string path;

			if (!(lineStream >> path))
				throw std::runtime_error("OBJ: syntax error while getting material texture");

			mat->texture_index = scene.getTextures().size();
			scene.addTexture(getFilePath(_filename) + path);
		}
		else if (identifier == "map_Ke")
		{
			std::string path;

			if (!(lineStream >> path))
				throw std::runtime_error("OBJ: syntax error while getting material texture");

			mat->emission_texture_index = scene.getEmissionTextures().size();
			std::cout << "path " << mat->emission_texture_index << " : " << getFilePath(_filename) + path << std::endl;
			scene.addEmissionTexture(getFilePath(_filename) + path);
		}
		else
			std::cerr << "unsupported material setting : " << identifier << std::endl;
	}
	if(mat)
	{
		scene.addMaterial(mat);
		_matNames[matName] = scene.getMaterialData().size() - 1;
	}
	file.close();
	for(auto i = _matNames.begin(); i != _matNames.end(); i++)
	{
		std::cout << "key : " << i->first << std::endl;
		std::cout << "value :" << i->second << std::endl;
	}
}

void	ObjParser::parse(Scene &scene, glm::vec3 offset, float scale, glm::mat4 transform)
{
	std::string				line;
	std::string				identifier;

	while (getline(_file, line))
	{
		try{
			if(line[0] == '#' || line.empty())
				continue;
			std::stringstream		lineStream(line);
			identifier = "";
			lineStream >> identifier;
			if(identifier == "v")
				_vertices.push_back(getVertex(lineStream));
			else if (identifier == "vt")
				_textureVertices.push_back(getUV(lineStream));
			else if (identifier == "f")
				addFace(lineStream);
			else if (identifier == "mtllib")
				parseMtl(lineStream, scene);
			else if (identifier == "usemtl")
			{
				lineStream >> identifier;
				if(_matNames.find(identifier) == _matNames.end())
					throw std::runtime_error("OBJ: invalid material name");
				_mat = _matNames[identifier];
			}
		} catch (std::exception &e)
		{
			std::cerr << line << std::endl;
			throw;
		}
	}

	scene.addBvh(_triangles, offset, scale, transform);
}

