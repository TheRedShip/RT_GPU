/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 13:59:57 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/15 13:59:57 by TheRed           ###   ########.fr       */
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

		void		update_camera_vectors();
		glm::mat4	get_view_matrix();

		void		process_movement(float xoffset, float yoffset, bool constrainPitch);

	private:

		glm::vec3	_position;
		glm::vec3	_forward;
		glm::vec3	_up;
		glm::vec3	_right;

		float		_pitch;
		float		_yaw;
};

#endif