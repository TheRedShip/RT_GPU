/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 18:46:13 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SCENE__HPP
# define RT_SCENE__HPP

# include "RT.hpp"

struct GPUObject
{
	alignas(16)	glm::vec3	position;

	alignas(16)	glm::vec3	color;
	float					emission;
	float					roughness;
	float					specular;

	float					radius; // sphere
	alignas(16) glm::vec3	normal; // plane

	alignas(16) glm::vec3	edge1;	//quad
	alignas(16) glm::vec3	edge2;	//quad

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

		const std::vector<GPUObject>	&getGPUData() const;
		Camera							*getCamera(void) const;

		Material						*getMaterial(int material_index);
		
	private:
		std::vector<Object *>	_objects;
		std::vector<GPUObject>	_gpu_objects;

		std::vector<Material *>	_materials;

		Camera					*_camera;
};

#endif