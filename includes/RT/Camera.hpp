/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 13:59:57 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 17:42:18 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_CAMERA__HPP
# define RT_CAMERA__HPP

# include "RT.hpp"

class Camera
{
	public:

		Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch);
		~Camera(void);


		void		update(float deltaTime);
		void		processMouse(float xoffset, float yoffset, bool constrainPitch);
		void		processKeyboard(bool forward, bool backward, bool left, bool right, bool up, bool down);
		
		glm::mat4	getViewMatrix();
		glm::vec3	getPosition();

		void		setPosition(glm::vec3 position);

	private:
		void		updateCameraVectors();

		glm::vec3	_position;
		glm::vec3	_forward;
		glm::vec3	_up;
		glm::vec3	_right;

		float		_pitch;
		float		_yaw;

		glm::vec3	_velocity;
    	glm::vec3	_acceleration;

		float _maxSpeed = 8.0f;
		float _acceleration_rate = 40.0f;
		float _deceleration_rate = 20.0f;
		float _sensitivity = 0.2f;
};

#endif