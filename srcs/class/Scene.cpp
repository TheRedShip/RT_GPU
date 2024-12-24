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

Camera		*Scene::getCamera(void) const
{
	return (_camera);
}

void		Scene::addObject(std::unique_ptr<Object> object)
{
	_objects.push_back(std::move(object));

	this->updateGPUData();
}

void		Scene::updateGPUData()
{
	_gpuObjects.clear();
	for (const auto& obj : _objects)
	{
		GPUObject gpuObj;
		gpuObj.position = obj->getPosition();
		gpuObj.color = obj->getMaterial().color;
		gpuObj.roughness = obj->getMaterial().roughness;
		gpuObj.specular = obj->getMaterial().specular;
		gpuObj.type = static_cast<int>(obj->getType());
		
		if (obj->getType() == Object::Type::SPHERE) {
			auto sphere = static_cast<const Sphere*>(obj.get());
			gpuObj.radius = sphere->getRadius();
		}
		
		_gpuObjects.push_back(gpuObj);
	}
}

const std::vector<GPUObject>&	Scene::getGPUData() const
{
	return (_gpuObjects);
}