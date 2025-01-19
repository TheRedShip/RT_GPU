/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/19 18:46:38 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SCENE__HPP
# define RT_SCENE__HPP

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

	int						left_index;	
	int						right_index;

	int						is_leaf;
	
	int						first_primitive;
	int						primitive_count;
};

class Sphere;
class Camera;

class Scene
{
	public:
		Scene();
		~Scene();

		bool							parseScene(char *name);

		void							addObject(Object *object);
		void							addMaterial(Material *material);
		
		void							updateLightAndObjects(int mat_id);
		std::set<int>					getGPULights();

		const std::vector<GPUObject>	&getObjectData() const;
		const std::vector<GPUTriangle>	&getTriangleData() const;
		
		std::vector<GPUMaterial>		&getMaterialData();
		std::vector<GPUBvh>				&getBVH();

		GPUVolume						&getVolume();
		GPUDebug						&getDebug();

		Camera							*getCamera(void) const;
		GPUMaterial						getMaterial(int material_index);
		
	private:
		std::vector<GPUBvh>			_gpu_bvh;

		std::vector<GPUObject>		_gpu_objects;
		std::vector<GPUTriangle>	_gpu_triangles;

		std::vector<GPUMaterial>	_gpu_materials;

		std::set<int>				_gpu_lights;

		GPUVolume					_gpu_volume;
		GPUDebug					_gpu_debug;

		Camera						*_camera;
};

#endif