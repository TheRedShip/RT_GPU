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

void		Scene::updateGPUData()
{

	_gpu_objects.clear();
	for (const auto& obj : _objects)
	{
		GPUObject gpu_obj;
		gpu_obj.position = obj->getPosition();
		gpu_obj.color = obj->getMaterial()->color;
		gpu_obj.roughness = obj->getMaterial()->roughness;
		gpu_obj.specular = obj->getMaterial()->specular;
		gpu_obj.type = static_cast<int>(obj->getType());
		
		if (obj->getType() == Object::Type::SPHERE)
		{
			auto sphere = static_cast<const Sphere*>(obj);
			gpu_obj.radius = sphere->getRadius();
		}
		
		std::cout << gpu_obj.position.x << " " << gpu_obj.position.y << " " << gpu_obj.position.z << " " << gpu_obj.radius << " " << gpu_obj.roughness << " " << gpu_obj.specular << std::endl;
		
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