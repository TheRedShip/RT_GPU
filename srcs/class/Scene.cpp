/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 18:40:17 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

Scene::Scene()
{
	_camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
}

Scene::~Scene()
{
	delete (_camera);
}

bool		Scene::parseScene(char *name)
{
	std::ifstream	file(name);
	std::string		line;

	if (!file.is_open())
	{
		std::cout << "Error opening the file" << std::endl;
		file.close();
		return (false);
	}

	SceneParser		scene_parser(this);

	while (std::getline(file, line))
	{
		if (!scene_parser.parseLine(line))
		{
			std::cerr << line << std::endl;
			file.close();
			return (false);
		}
	}
	file.close();
	return (true);
}


void		Scene::addObject(Object *object)
{
	_objects.push_back(object);
	this->updateGPUData();
}

void		Scene::addMaterial(Material *material)
{
	_materials.push_back(material);
	this->updateGPUData();
}

void		Scene::updateGPUData()
{
	GPUObject	gpu_obj;
	GPUMaterial	gpu_mat;

	_gpu_objects.clear();
	_gpu_materials.clear();
	
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		Object *obj = _objects[i];

		gpu_obj.mat_index = obj->getMaterialIndex();
		gpu_obj.position = obj->getPosition();
		gpu_obj.type = static_cast<int>(obj->getType());
		
		if (obj->getType() == Object::Type::SPHERE)
		{
			auto sphere = static_cast<Sphere *>(obj);
			gpu_obj.radius = sphere->getRadius();
		}
		else if (obj->getType() == Object::Type::PLANE)
		{
			auto plane = static_cast<Plane *>(obj);
			gpu_obj.normal = plane->getNormal();
		}
		else if (obj->getType() == Object::Type::QUAD)
		{
			auto quad = static_cast<Quad *>(obj);
			gpu_obj.vertex1 = quad->getEdge1();
			gpu_obj.vertex2 = quad->getEdge2();
		}
		else if (obj->getType() == Object::Type::TRIANGLE)
		{
			auto triangle = static_cast<Triangle *>(obj);
			gpu_obj.vertex1 = triangle->getVertex2();
			gpu_obj.vertex2 = triangle->getVertex3();
			gpu_obj.normal = triangle->getNormal();
		}
		else if (obj->getType() == Object::Type::CUBE)
		{
			auto cube = static_cast<Cube *>(obj);
			gpu_obj.position = cube->getPosition();
			gpu_obj.vertex1 = cube->getSize();
			gpu_obj.type = static_cast<int>(cube->getType());
		}
		else if (obj->getType() == Object::Type::PORTAL)
		{
			auto portal = static_cast<Portal *>(obj);
			gpu_obj.vertex1 = portal->getEdge1();
			gpu_obj.vertex2 = portal->getEdge2();
			gpu_obj.normal = portal->getNormal();
			gpu_obj.transform = glm::mat4(portal->getTransform());
			
			Portal *linked = static_cast<Portal *>(_objects[i - 2]);

			if (portal->getLinkedPortalIndex() == -1)
			{
				if (linked->getType() == Object::Type::PORTAL && linked->getLinkedPortalIndex() == static_cast<int>(i))
					portal->setLinkedPortalIndex(i - 2);
				else
					portal->setLinkedPortalIndex(i + 2);
			}

			gpu_obj.radius = portal->getLinkedPortalIndex();
		}

		_gpu_objects.push_back(gpu_obj);
	}
	for (const auto &material : _materials)
	{
		gpu_mat.color = material->color;
		gpu_mat.emission = material->emission;
		gpu_mat.roughness = material->roughness;
		gpu_mat.metallic = material->metallic;
		gpu_mat.type = material->type;

		_gpu_materials.push_back(gpu_mat);
	}
}

const std::vector<GPUObject>&	Scene::getObjectData() const
{
	return (_gpu_objects);
}

const std::vector<GPUMaterial>&	Scene::getMaterialData() const
{
	return (_gpu_materials);
}

Camera		*Scene::getCamera(void) const
{
	return (_camera);
}

Material	*Scene::getMaterial(int material_index)
{
	if (material_index < 0 || material_index >= (int)_materials.size())
		throw std::runtime_error("Incorrect material index");
	return (_materials[material_index]);
}