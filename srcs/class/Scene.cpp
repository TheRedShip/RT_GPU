/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/20 18:42:50 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

Scene::Scene()
{
	_camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
	
	_gpu_volume.enabled = 0;
	_gpu_volume.sigma_a = glm::vec3(0.0000f);
	_gpu_volume.sigma_s = glm::vec3(0.0800f);
	_gpu_volume.sigma_t = _gpu_volume.sigma_a + _gpu_volume.sigma_s;
	_gpu_volume.g = 1.0f;

	_gpu_debug.enabled = 0;
	_gpu_debug.mode = 0;	
	_gpu_debug.triangle_treshold = 1;
	_gpu_debug.box_treshold = 1;
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


	std::cout << "Parsing done" << std::endl;

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
	else if (obj->getType() == Object::Type::TRIANGLE)
	{
		GPUTriangle	gpu_triangle;

		auto triangle = static_cast<Triangle *>(obj);
		gpu_triangle.position = triangle->getPosition();
		gpu_triangle.mat_index = triangle->getMaterialIndex();

		gpu_triangle.vertex1 = triangle->getVertex2();
		gpu_triangle.vertex2 = triangle->getVertex3();
		gpu_triangle.normal = triangle->getNormal();

		_gpu_triangles.push_back(gpu_triangle);
		return ;
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

void		Scene::addBvh(std::vector<Triangle> &triangles, glm::vec3 offset)
{
	GPUBvhData			new_bvh_data;
	std::vector<GPUBvh>	new_bvhs_list;

	uint64_t start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	
	std::cout << "New BVH" << std::endl;

	BVH *bvh = new BVH(triangles, 0, triangles.size());
	new_bvhs_list = bvh->getGPUBvhs();
	
	std::cout << glm::to_string(offset) << std::endl;
	new_bvh_data.offset = offset;
	new_bvh_data.bvh_start_index = _gpu_bvh.size();
	new_bvh_data.triangle_start_index = _gpu_triangles.size();
	
	_gpu_bvh_data.push_back(new_bvh_data);
	_gpu_bvh.insert(_gpu_bvh.end(), new_bvhs_list.begin(), new_bvhs_list.end());

	for (int i = 0; i < (int)triangles.size(); i++)
	{
		GPUTriangle	gpu_triangle;

		gpu_triangle.position = triangles[i].getPosition();
		gpu_triangle.mat_index = triangles[i].getMaterialIndex();

		gpu_triangle.vertex1 = triangles[i].getVertex2();
		gpu_triangle.vertex2 = triangles[i].getVertex3();
		gpu_triangle.normal = triangles[i].getNormal();

		_gpu_triangles.push_back(gpu_triangle);
	}

	uint64_t time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start_time;
	std::cout << "\tBuild done in " << time_elapsed << "ms" <<  std::endl;

	std::cout << "\tBVH size: " << bvh->getSize() << std::endl;
	std::cout << "\tBVH leaves: " << bvh->getLeaves() << std::endl << std::endl;

	BVHStats stats = bvh->analyzeBVHLeaves(bvh);
	std::cout << "\tMin triangles per leaf: " << stats.min_triangles << std::endl;
	std::cout << "\tMax triangles per leaf: " << stats.max_triangles << std::endl;
	std::cout << "\tAverage triangles per leaf: " << stats.average_triangles << std::endl << std::endl;
	
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

std::set<int>					Scene::getGPULights()
{
	return (_gpu_lights);
}

const std::vector<GPUObject>	&Scene::getObjectData() const
{
	return (_gpu_objects);
}

const std::vector<GPUTriangle>	&Scene::getTriangleData() const
{
	return (_gpu_triangles);
}

std::vector<GPUMaterial>		&Scene::getMaterialData()
{
	return (_gpu_materials);
}

GPUVolume						&Scene::getVolume()
{
	return (_gpu_volume);
}

GPUDebug						&Scene::getDebug()
{
	return (_gpu_debug);
}

std::vector<GPUBvhData>				&Scene::getBvhData()
{
	return (_gpu_bvh_data);
}

std::vector<GPUBvh>				&Scene::getBvh()
{
	return (_gpu_bvh);
}

Camera							*Scene::getCamera(void) const
{
	return (_camera);
}

GPUMaterial	Scene::getMaterial(int material_index)
{
	if (material_index < 0 || material_index >= (int)_gpu_materials.size())
		throw std::runtime_error("Incorrect material index");
	return (_gpu_materials[material_index]);
}
