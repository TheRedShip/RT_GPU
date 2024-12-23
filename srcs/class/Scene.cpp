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
	delete _camera;
}

Camera		*Scene::getCamera(void) const
{
	return (_camera);
}