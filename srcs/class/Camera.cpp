/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:00:38 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/06 19:45:46 by ycontre          ###   ########.fr       */
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

	std::cout << _yaw << std::endl;

	frontTemp.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	frontTemp.y = sin(glm::radians(_pitch));
	frontTemp.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(frontTemp);

	_right = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	_up = glm::normalize(glm::cross(_right, _forward));
}

void Camera::updateCameraDirections()
{
    float new_yaw = glm::degrees(atan2(-_forward.x, _forward.z));
    if (new_yaw < 0.0f)
        new_yaw += 360.0f;

    float old_yaw_mod = fmod(_yaw, 360.0f);
    if (old_yaw_mod < 0.0f)
        old_yaw_mod += 360.0f;

    float delta_yaw = new_yaw - old_yaw_mod;
    if (delta_yaw > 180.0f)
        delta_yaw -= 360.0f;
    else if (delta_yaw < -180.0f)
        delta_yaw += 360.0f;

    _yaw += delta_yaw + 90.0f;

    _pitch = glm::degrees(asin(_forward.y));
    _pitch = glm::clamp(_pitch, -89.0f, 89.0f);
}

void		Camera::update(Scene *scene, float delta_time)
{
	// delta_time = std::min(delta_time, 0.01f);

	_velocity += _acceleration * delta_time;

	if (glm::length(_acceleration) < 0.1f)
		_velocity *= std::max(0.0f, 1.0f - _deceleration_rate * delta_time);
	
	float speed = glm::length(_velocity);
	if (speed > _maxSpeed)
		_velocity = glm::normalize(_velocity) * _maxSpeed;
	
	if (glm::length(_velocity) > 0.0f && !this->portalTeleport(scene, delta_time))
		_position += _velocity * delta_time;

	_acceleration = glm::vec3(0.0f);
}

int	Camera::portalTeleport(Scene *scene, float delta_time)
{
	for (const GPUObject &obj : scene->getObjectData())
	{
		if (obj.type != (int)Object::Type::PORTAL)
			continue;

		glm::vec3 portal_to_camera = _position - obj.position;
		float distance_plane = glm::dot(portal_to_camera, obj.normal);
		glm::vec3 point_projected = _position - distance_plane * obj.normal;

		glm::mat2 A = glm::mat2(
			glm::dot(obj.vertex1, obj.vertex1), glm::dot(obj.vertex1, obj.vertex2),
			glm::dot(obj.vertex1, obj.vertex2), glm::dot(obj.vertex2, obj.vertex2)
		);
		glm::vec2 b = glm::vec2(
			glm::dot(point_projected - obj.position, obj.vertex1),
			glm::dot(point_projected - obj.position, obj.vertex2)
		);
		glm::vec2 alphaBeta = glm::inverse(A) * b;

		if (alphaBeta.x >= 0.0f && alphaBeta.x <= 1.0f && alphaBeta.y >= 0.0f && alphaBeta.y <= 1.0f)
		{
			glm::vec3 future_pos = _position + _velocity * delta_time;

			float distance_future_pos = glm::length(future_pos - _position);
			float distance_portal = glm::length(point_projected - _position);

			float imprecision = 0.1f;
			if (distance_portal <= distance_future_pos && glm::dot(glm::normalize(future_pos - _position), obj.normal) > 0.0f)
			{
				std::cout << "Teleport" << std::endl;
				
				GPUObject linked_portal = scene->getObjectData()[obj.radius];

				glm::mat3 portal_transform = glm::mat3(linked_portal.transform) * glm::inverse(glm::mat3(obj.transform));

				if (dot(obj.normal, linked_portal.normal) > 0.0)
				{
					glm::mat3 reflection = glm::mat3(1.0) - 2.0f * glm::outerProduct(linked_portal.normal, linked_portal.normal);
					portal_transform *= reflection;
				}

				//teleportation

				glm::vec3 relative_pos = _position - obj.position;
            	glm::vec3 transformed_relative_pos = portal_transform * relative_pos;
	
				float remaining_distance = distance_future_pos - distance_portal + imprecision;
				glm::vec3 new_movement = remaining_distance * portal_transform * linked_portal.normal;
            
 				_position = linked_portal.position + transformed_relative_pos - new_movement;
				// _position = (linked_portal.position) + (_position - obj.position) - (((distance_future_pos - distance_portal + imprecision)) * linked_portal.normal);

				// view direction

				_forward = glm::vec3(portal_transform * glm::vec4(_forward, 1.0f));
				_up = glm::vec3(portal_transform * glm::vec4(_up, 1.0f));
				_right = glm::vec3(portal_transform * glm::vec4(_right, 1.0f));

				updateCameraDirections();

				_velocity = glm::vec3(portal_transform * glm::vec4(_velocity, 0.0f));

				return (1);
			}
		}
	}

	return (0);
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
