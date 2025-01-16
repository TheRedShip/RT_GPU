/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/15 19:34:49 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

Scene::Scene()
{
	_camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
	
	_gpu_volume.enabled = 0;
	_gpu_volume.sigma_a = glm::vec3(0.0001f);
	_gpu_volume.sigma_s = glm::vec3(0.0800f);
	_gpu_volume.sigma_t = _gpu_volume.sigma_a + _gpu_volume.sigma_s;
	_gpu_volume.g = 0.9f;
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

	//bvh
	BVH		*bvh = new BVH(_gpu_objects, 0, _gpu_objects.size());
	bvh->showAABB(this);
	// addObject(new Cube((bvh->getAABB().max + bvh->getAABB().min) / 2.0f, bvh->getAABB().max - bvh->getAABB().min, 7));
	//

	return (true);
}


void		Scene::addObject(Object *obj)
{
	GPUObject	gpu_obj;

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
		gpu_obj.vertex1 = quad->getUp();
		gpu_obj.vertex2 = quad->getRight();
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
	}
	else if (obj->getType() == Object::Type::CYLINDER)
	{
		auto cylinder = static_cast<Cylinder *>(obj);
		gpu_obj.normal = glm::vec3(cylinder->getRadius(), cylinder->getHeight(), 0.0f);
		gpu_obj.transform = glm::mat4(cylinder->getRotation());
	}
	else if (obj->getType() == Object::Type::PORTAL)
	{
		auto portal = static_cast<Portal *>(obj);
		gpu_obj.vertex1 = portal->getUp();
		gpu_obj.vertex2 = portal->getRight();
		gpu_obj.normal = portal->getNormal();
		gpu_obj.transform = glm::mat4(portal->getRotation());

		int	i = _gpu_objects.size();
		
		GPUObject	&linked = _gpu_objects[i - 2];

		if (portal->getLinkedPortalIndex() == -1)
		{
			if (linked.type == (int)Object::Type::PORTAL && linked.radius == static_cast<int>(i))
				portal->setLinkedPortalIndex(i - 2);
			else
				portal->setLinkedPortalIndex(i + 2);
		}

		gpu_obj.radius = portal->getLinkedPortalIndex();
	}

	_gpu_objects.push_back(gpu_obj);
}

void		Scene::addMaterial(Material *material)
{
	GPUMaterial	gpu_mat;

	gpu_mat.color = material->color;
	gpu_mat.emission = material->emission;
	gpu_mat.roughness = material->roughness;
	gpu_mat.metallic = material->metallic;
	gpu_mat.refraction = material->refraction;
	gpu_mat.type = material->type;

	_gpu_materials.push_back(gpu_mat);
}

void		Scene::updateLightAndObjects(int mat_id)
{
	for (unsigned int i = 0; i < _gpu_objects.size(); i++)
	{
		if (_gpu_objects[i].mat_index == mat_id)
			_gpu_lights.insert(i);
	}
	for (auto it = _gpu_lights.begin(); it != _gpu_lights.end(); )
	{
        if (_gpu_materials[_gpu_objects[*it].mat_index].emission <= 0.0) 
            it = _gpu_lights.erase(it);
		else
            ++it;
    }
}

std::set<int>				Scene::getGPULights()
{
	return (_gpu_lights);
}

const std::vector<GPUObject>&	Scene::getObjectData() const
{
	return (_gpu_objects);
}

std::vector<GPUMaterial>&	Scene::getMaterialData()
{
	return (_gpu_materials);
}

GPUVolume	&Scene::getVolume()
{
	return (_gpu_volume);
}

Camera		*Scene::getCamera(void) const
{
	return (_camera);
}

GPUMaterial	Scene::getMaterial(int material_index)
{
	if (material_index < 0 || material_index >= (int)_gpu_materials.size())
		throw std::runtime_error("Incorrect material index");
	return (_gpu_materials[material_index]);
}