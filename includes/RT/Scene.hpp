/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/10 18:58:38 by ycontre          ###   ########.fr       */
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

	alignas(16) glm::vec3	vertex1;	//quad triangle
	alignas(16) glm::vec3	vertex2;	//quad triangle

	float					radius; // sphere

	int						mat_index;
	int						type;

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

		void							updateGPUData();

		const std::vector<GPUObject>	&getObjectData() const;
		std::vector<GPUMaterial>		&getMaterialData();

		Camera							*getCamera(void) const;
		Material						*getMaterial(int material_index);
		
	private:
		std::vector<Object *>		_objects;
		std::vector<Material *>		_materials;

		std::vector<GPUObject>		_gpu_objects;
		std::vector<GPUMaterial>	_gpu_materials;


		Camera						*_camera;
};

#endif