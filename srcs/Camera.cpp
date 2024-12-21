/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:00:38 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/15 14:00:38 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Camera.hpp"

Camera::Camera(glm::vec3 start_pos, glm::vec3 start_up, float start_yaw, float start_pitch)
			: _forward(glm::vec3(0.0f, 0.0f, -1.0f)), _yaw(start_yaw), _pitch(start_pitch), _position(start_pos), _up(start_up)
{
	update_camera_vectors();
}

Camera::~Camera(void)
{
}

void		Camera::update_camera_vectors()
{
	glm::vec3 frontTemp;

	frontTemp.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	frontTemp.y = sin(glm::radians(_pitch));
	frontTemp.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(frontTemp);

	_right = glm::normalize(glm::cross(_forward, _up));
	_up = glm::normalize(glm::cross(_right, _forward));
}

glm::mat4	Camera::get_view_matrix()
{
	return (glm::lookAt(_position, _position + _forward, _up));
}

void		Camera::process_movement(float xoffset, float yoffset, bool constraint_pitch = true)
{
	_yaw   += xoffset;
	_pitch += yoffset;

	if (constraint_pitch)
	{
		if (_pitch > 89.0f) _pitch = 89.0f;
		if (_pitch < -89.0f) _pitch = -89.0f;
	}

	update_camera_vectors();

	std::cout << _yaw << " " << _pitch << std::endl;
}