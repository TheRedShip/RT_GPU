/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:00:38 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/28 02:15:41 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Camera.hpp"

Camera::Camera(glm::vec3 start_pos, glm::vec3 start_up, float start_yaw, float start_pitch)
			:  _position(start_pos), _forward(glm::vec3(0.0f, 0.0f, -1.0f)), _up(start_up), _pitch(start_pitch), _yaw(start_yaw),
				_velocity(0.0f), _acceleration(0.0f)
{
	updateCameraVectors();
}

Camera::~Camera(void)
{
}

void		Camera::updateCameraVectors()
{
	glm::vec3 frontTemp;

	frontTemp.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	frontTemp.y = sin(glm::radians(_pitch));
	frontTemp.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(frontTemp);

	_right = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	_up = glm::normalize(glm::cross(_right, _forward));
}

void		Camera::update(float delta_time)
{
	// delta_time = std::min(delta_time, 0.01f);

	_velocity += _acceleration * delta_time;

    if (glm::length(_acceleration) < 0.1f)
        _velocity *= std::max(0.0f, 1.0f - _deceleration_rate * delta_time);
    
    float speed = glm::length(_velocity);
    if (speed > _maxSpeed)
        _velocity = glm::normalize(_velocity) * _maxSpeed;
    
    _position += _velocity * delta_time;
    _acceleration = glm::vec3(0.0f);
}


void		Camera::processMouse(float xoffset, float yoffset, bool constraint_pitch = true)
{
	_yaw   += xoffset * _sensitivity;
	_pitch += yoffset * _sensitivity;

//	while(_yaw < 0)
//		_yaw += 360;
//	while(_yaw > 360)
//		_yaw -= 360;
	if (constraint_pitch)
	{
		if (_pitch > 89.0f) _pitch = 89.0f;
		if (_pitch < -89.0f) _pitch = -89.0f;
	}

	updateCameraVectors();
}

void		Camera::processKeyboard(bool forward, bool backward, bool left, bool right, bool up, bool down)
{
	glm::vec3 acceleration(0.0f);

	if (forward)  acceleration += _forward;
    if (backward) acceleration -= _forward;
    if (right)    acceleration += _right;
    if (left)     acceleration -= _right;
    if (up)       acceleration += _up;
    if (down)     acceleration -= _up;
    
    if (glm::length(acceleration) > 0.1f)
        acceleration = glm::normalize(acceleration) * _acceleration_rate;
    
    _acceleration = acceleration;
}

glm::mat4	Camera::getViewMatrix()
{
	return (glm::lookAt(_position, _position + _forward, _up));
}

glm::vec3	Camera::getPosition()
{
	return (_position);
}

glm::vec2	Camera::getDirection()
{
	return (glm::vec2(_pitch, _yaw));
}

GPUCamera	Camera::getGPUData()
{
	GPUCamera data;

	data.view_matrix = getViewMatrix();
	data.camera_position = _position;
	
	data.aperture_size = _aperture_size;
	data.focus_distance = _focus_distance;
	data.fov = _fov;

	data.bounce = _bounce;

	return (data);
}

float		Camera::getVelocity()
{
	return (glm::length(_velocity));
}

int			&Camera::getBounce()
{
	return (_bounce);
}

float		&Camera::getFov()
{
	return (_fov);
}

float		&Camera::getAperture()
{
	return (_aperture_size);
}

float		&Camera::getFocus()
{
	return (_focus_distance);
}

void		Camera::setPosition(glm::vec3 position)
{
	_position = position;
}

void		Camera::setDirection(float pitch, float yaw)
{
	_pitch = pitch;
	_yaw = yaw;
	updateCameraVectors();
}

void		Camera::setDOV(float aperture, float focus)
{
	_aperture_size = aperture;
	_focus_distance = focus;
}

void		Camera::setBounce(int bounce)
{
	_bounce = bounce;
}

void		Camera::setFov(float fov)
{
	_fov = fov;
}
