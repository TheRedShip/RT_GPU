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
}

void		Scene::updateGPUData()
{
	GPUObject	gpu_obj;
	Material	*mat;

	_gpu_objects.clear();
	for (const auto& obj : _objects)
	{
		mat = getMaterial(obj->getMaterialIndex());
		
		gpu_obj.position = obj->getPosition();

		gpu_obj.color = mat->color;
		gpu_obj.emission = mat->emission;
		gpu_obj.roughness = mat->roughness;
		gpu_obj.specular = mat->specular;
		
		gpu_obj.type = static_cast<int>(obj->getType());
		
		if (obj->getType() == Object::Type::SPHERE)
		{
			auto sphere = static_cast<const Sphere *>(obj);
			gpu_obj.radius = sphere->getRadius();
		}
		else if (obj->getType() == Object::Type::PLANE)
		{
			auto plane = static_cast<const Plane *>(obj);
			gpu_obj.normal = plane->getNormal();
		}

		_gpu_objects.push_back(gpu_obj);
	}
}

const std::vector<GPUObject>&	Scene::getGPUData() const
{
	return (_gpu_objects);
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