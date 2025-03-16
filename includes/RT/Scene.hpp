/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/16 17:00:32 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "RT.hpp"

struct GPUObject
{
	glm::mat4				transform;

	alignas(16)	glm::vec3	position;

	alignas(16) glm::vec3	normal; // plane triangle

	alignas(16) glm::vec3	vertex1;//quad triangle
	alignas(16) glm::vec3	vertex2;//quad triangle

	float					radius; // sphere

	int						mat_index;
	int						type;

};

struct GPUTriangle
{
	alignas(16) glm::vec3	position;
	alignas(16) glm::vec3	vertex1;
	alignas(16) glm::vec3	vertex2;
	alignas(16) glm::vec3	normal;

	alignas(8) glm::vec2	texture_vertex1;
	alignas(8) glm::vec2	texture_vertex2;
	alignas(8) glm::vec2	texture_vertex3;

	int						mat_index;
};

struct GPUMaterial
{
	alignas(16)	glm::vec3	color;
	float					emission;
	float					roughness;
	float					metallic;
	float					refraction;
	int						type;
	int						texture_index;
	int						emission_texture_index;
};

struct GPUVolume
{
	alignas(16) glm::vec3	sigma_a;
	alignas(16) glm::vec3	sigma_s;
	alignas(16) glm::vec3	sigma_t;

	float					g;

	int						enabled;
};

struct GPUDebug
{
	int	enabled;
	int	mode;
	int	triangle_treshold;
	int	box_treshold;
};

struct GPUBvh
{
	alignas(16) glm::vec3	min;
	alignas(16) glm::vec3	max;

	int						index;
	int						primitive_count;
	
};

struct GPUBvhData
{
	glm::mat4				transform;
	glm::mat4				inv_transform;
	alignas(16) glm::vec3	offset;
	float					scale;

	int						bvh_start_index;
	int						triangle_start_index;
};

class Sphere;
class Camera;

class Scene
{
	public:
		Scene(std::string &name);
		~Scene();

		void							addObject(Object *object);
		void							addMaterial(Material *material);
		void							addTexture(std::string path);
		void							addEmissionTexture(std::string path);

		bool							loadTextures();
		
		void							updateLightAndObjects(int mat_id);
		std::set<int>					getGPULights();

		void							addBvh(std::vector<Triangle> &triangles, glm::vec3 offset, float scale, glm::mat4 transform);

		const std::vector<GPUObject>	&getObjectData() const;
		const std::vector<GPUTriangle>	&getTriangleData() const;
		
		std::vector<GPUMaterial>		&getMaterialData();
		std::vector<GLuint>				&getTextureIDs();
		std::vector<GLuint>				&getEmissionTextureIDs();

		std::vector<std::string>		&getTextures();
		std::vector<std::string>		&getEmissionTextures();

		std::vector<GPUBvhData>			&getBvhData();
		std::vector<GPUBvh>				&getBvh();

		GPUVolume						&getVolume();
		GPUDebug						&getDebug();
		GPUDenoise						&getDenoise();

		Camera							*getCamera(void) const;
		GPUMaterial						getMaterial(int material_index);
		
		bool							fail(void) const;

		void							changeScene(std::string &name);

		std::vector<Buffer *>			createDataOnGPU(void);
		
	private:
		void						init(std::string &name);

		bool						_fail;
		std::vector<GPUBvhData>		_gpu_bvh_data;
		std::vector<GPUBvh>			_gpu_bvh;

		std::vector<GPUObject>		_gpu_objects;
		std::vector<GPUTriangle>	_gpu_triangles;

		std::vector<GPUMaterial>	_gpu_materials;

		std::vector<std::string>	_textures;
		std::vector<std::string>	_emissive_textures;

		std::vector<GLuint>			_gpu_textures;
		std::vector<GLuint>			_gpu_emissive_textures;

		std::set<int>				_gpu_lights;

		GPUVolume					_gpu_volume;
		GPUDebug					_gpu_debug;
		GPUDenoise					_gpu_denoise;

		Camera						*_camera;
};
