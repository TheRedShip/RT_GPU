/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/18 13:37:16 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Scene::Scene(std::string &name)
{
	_camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
	_error = 0;
	init(name);
}

void Scene::init(std::string &name)
{
	std::ifstream	file(name);
	std::string		line;

	
	_gpu_volume.enabled = 0;
	_gpu_volume.sigma_a = glm::vec3(0.0000f);
	_gpu_volume.sigma_s = glm::vec3(0.0800f);
	_gpu_volume.sigma_t = _gpu_volume.sigma_a + _gpu_volume.sigma_s;
	_gpu_volume.g = 1.0f;

	_gpu_debug.enabled = 0;
	_gpu_debug.mode = 0;	
	_gpu_debug.triangle_treshold = 1;
	_gpu_debug.box_treshold = 1;

	_gpu_denoise.enabled = 0;
	_gpu_denoise.pass = 0;
	_gpu_denoise.c_phi = 0.4f;
	_gpu_denoise.p_phi = 0.1f;
	_gpu_denoise.n_phi = 0.1f;

	if (!file.is_open())
	{
		std::cerr << "Can't open scene file" << std::endl;
		_error = 1;
		return ;
	}

	SceneParser		scene_parser(this, name);

	while (std::getline(file, line))
	{
		if (!scene_parser.parseLine(line))
		{
			file.close();
			std::cerr << line << std::endl;
			_error = 1;
			return ;
		}
	}
	file.close();


	std::cout << "Parsing done" << std::endl;
}

Scene::~Scene()
{
	delete (_camera);
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
		gpu_obj.normal = quad->getNormal();
		gpu_obj.radius = quad->getSingleSided();
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
		auto triangle = static_cast<Triangle *>(obj);

		gpu_obj.vertex1 = triangle->getVertex2();
		gpu_obj.vertex2 = triangle->getVertex3();
		gpu_obj.normal = triangle->getNormal();
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

void		Scene::addBvh(std::vector<Triangle> &triangles, glm::vec3 offset, float scale, glm::mat4 transform)
{
	GPUBvhData			new_bvh_data;
	std::vector<GPUBvh>	new_bvhs_list;

	uint64_t start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	
	std::cout << "New BVH" << std::endl;

	BVH *bvh = new BVH(triangles, 0, triangles.size());
	new_bvhs_list = bvh->getGPUBvhs();
	
	new_bvh_data.transform = transform * scale;
	new_bvh_data.inv_transform = glm::inverse(new_bvh_data.transform);
	new_bvh_data.offset = offset;
	new_bvh_data.scale = scale;
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

		gpu_triangle.texture_vertex1 = triangles[i].getTextureVertex1();
		gpu_triangle.texture_vertex2 = triangles[i].getTextureVertex2();
		gpu_triangle.texture_vertex3 = triangles[i].getTextureVertex3();

		gpu_triangle.normal_vertex1 = triangles[i].getNormalVertex1();
		gpu_triangle.normal_vertex2 = triangles[i].getNormalVertex2();
		gpu_triangle.normal_vertex3 = triangles[i].getNormalVertex3();

		_gpu_triangles.push_back(gpu_triangle);
	}

	uint64_t time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start_time;
	std::cout << "\tBuild done in " << time_elapsed << "ms" <<  std::endl;

	std::cout << "\tBVH size: " << bvh->getSize() << std::endl;
	std::cout << "\tBVH leaves: " << bvh->getLeaves() << std::endl << std::endl;

	BVHStats stats = bvh->analyzeBVHLeaves(bvh, 0);
	std::cout << "\tMin triangles per leaf: " << stats.min_triangles << std::endl;
	std::cout << "\tMax triangles per leaf: " << stats.max_triangles << std::endl;
	std::cout << "\tAverage triangles per leaf: " << stats.average_triangles << std::endl << std::endl;

	std::cout << "\n\tMin depth: " << stats.min_depth << std::endl;
	std::cout << "\tMax depth: " << stats.max_depth << std::endl;
	std::cout << "\tAverage depth: " << stats.average_depth << std::endl;

	
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
	gpu_mat.texture_index = material->texture_index;
	gpu_mat.emission_texture_index = material->emission_texture_index;

	_gpu_materials.push_back(gpu_mat);
}
void		Scene::addTexture(std::string path)
{
	_textures.push_back(path);
}

void		Scene::addEmissionTexture(std::string path)
{
	_emissive_textures.push_back(path);
}

bool		Scene::loadTextures()
{
	for (std::string &path : _textures)
	{
		int width, height, channels;
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		
		if (!image)
		{
			std::cout << "Failed to load texture " << path << std::endl;
			return (false);
		}

		
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		
		std::cout << "Loaded texture: (" << textureID << "): " << path << " (" << width << "x" << height << ")" << std::endl;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		_gpu_textures.push_back(textureID);

		stbi_image_free(image);
	}

	for (std::string &path : _emissive_textures)
	{
		int width, height, channels;
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		
		if (!image)
		{
			std::cout << "Failed to load texture " << path << std::endl;
			return (false);
		}

		
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		
		std::cout << "Loaded emissive texture (" << textureID << "): " << path << " (" << width << "x" << height << ")" << std::endl;
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		_gpu_emissive_textures.push_back(textureID);

		stbi_image_free(image);
	}

	return (true);
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

bool		Scene::error(void) const
{
	return(_error);
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

std::vector<GLuint>				&Scene::getTextureIDs()
{
	return (_gpu_textures);
}

std::vector<GLuint>				&Scene::getEmissionTextureIDs()
{
	return (_gpu_emissive_textures);
}

std::vector<std::string>		&Scene::getTextures()
{
	return (_textures);
}

std::vector<std::string>		&Scene::getEmissionTextures()
{
	return (_emissive_textures);
}

GPUVolume						&Scene::getVolume()
{
	return (_gpu_volume);
}

GPUDebug						&Scene::getDebug()
{
	return (_gpu_debug);
}

GPUDenoise						&Scene::getDenoise()
{
	return (_gpu_denoise);
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

std::vector<Buffer *>	Scene::createDataOnGPU(void)
{
	GLint max_gpu_size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_gpu_size);


	std::cout << "Sending " << _gpu_objects.size() << " objects for " << \
				_gpu_objects.size() * sizeof(GPUObject) + \
				_gpu_triangles.size() * sizeof(GPUTriangle) + \
				_gpu_bvh.size() * sizeof(GPUBvh) + \
				_gpu_materials.size() * sizeof(GPUMaterial) \
				<< " / " << max_gpu_size << " bytes" << std::endl;

	std::vector<Buffer *> buffers;

	buffers.push_back(new Buffer(Buffer::Type::SSBO, 1, sizeof(GPUObject) * _gpu_objects.size(), _gpu_objects.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 2, sizeof(GPUTriangle) * _gpu_triangles.size(), _gpu_triangles.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 3, sizeof(GPUBvhData) * _gpu_bvh_data.size(), _gpu_bvh_data.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 4, sizeof(GPUBvh) * _gpu_bvh.size(), _gpu_bvh.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 5, sizeof(GPUMaterial) * _gpu_materials.size(), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 6, getGPULights().size() * sizeof(int), nullptr));

	buffers.push_back(new Buffer(Buffer::Type::UBO, 0, sizeof(GPUCamera), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::UBO, 1, sizeof(GPUVolume), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::UBO, 2, sizeof(GPUDebug), nullptr));

	return (buffers);
}

void		Scene::changeScene(std::string &name, std::vector<Buffer *> &buffers)
{
	_gpu_bvh_data.clear();
	_gpu_bvh.clear();
	_gpu_objects.clear();
	_gpu_triangles.clear();
	_gpu_materials.clear();
	_textures.clear();
	_emissive_textures.clear();
	_gpu_textures.clear();
	_gpu_emissive_textures.clear();
	_gpu_lights.clear();
	for (size_t i = 0; i < buffers.size(); i++)
		delete buffers[i];
	buffers.clear();

	init(name);
	buffers = createDataOnGPU();
	loadTextures();
}
