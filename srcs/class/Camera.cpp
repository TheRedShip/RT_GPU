/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:00:38 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 17:42:20 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Camera.hpp"

Camera::Camera(glm::vec3 start_pos, glm::vec3 start_up, float start_yaw, float start_pitch)
			: _forward(glm::vec3(0.0f, 0.0f, -1.0f)), _position(start_pos), _up(start_up), _pitch(start_pitch), _yaw(start_yaw)
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

	_right = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	_up = glm::normalize(glm::cross(_right, _forward));
}

glm::mat4	Camera::get_view_matrix()
{
	return (glm::lookAt(_position, _position + _forward, _up));
}

glm::vec3	Camera::get_position()
{
	return (_position);
}

void		Camera::process_mouse(float xoffset, float yoffset, bool constraint_pitch = true)
{
	_yaw   += xoffset * 0.2f;
	_pitch += yoffset * 0.2f;

	if (constraint_pitch)
	{
		if (_pitch > 89.0f) _pitch = 89.0f;
		if (_pitch < -89.0f) _pitch = -89.0f;
	}

	update_camera_vectors();
}

void		Camera::process_keyboard(bool forward, bool backward, bool left, bool right, bool up, bool down)
{
	float	speed;

	speed = .1f;

	if (forward) _position += _forward * speed;
	if (backward) _position -= _forward * speed;
	if (left) _position -= _right * speed;
	if (right) _position += _right * speed;
	if (up) _position += _up * speed;
	if (down) _position -= _up * speed;
}