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
	glm::vec3	position;
	int			padding_1;
	glm::vec3	color;
	int			padding_2;
	float		roughness;
	float		specular;
	float		radius;
	int			type;
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

		void							updateGPUData();

		const std::vector<GPUObject>	&getGPUData() const;
		Camera							*getCamera(void) const;
		
	private:
		std::vector<Object *>	_objects;
		std::vector<GPUObject>	_gpu_objects;

		Camera					*_camera;
};

#endif