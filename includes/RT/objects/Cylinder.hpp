/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cylinder.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/08 20:20:34 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_CYLINDER__HPP
# define RT_CYLINDER__HPP

# include "RT.hpp"

class Cylinder : public Object
{
	public:
		Cylinder(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z;
				float	radius, height;
				float	pitch, yaw, roll;

				int		mat_index;

				if (!(line >> x >> y >> z))
					throw std::runtime_error("Missing position");

				if (!(line >> radius >> height))
					throw std::runtime_error("Missing radius or height values");

				if (!(line >> pitch >> yaw >> roll))
					throw std::runtime_error("Missing rotation values");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);

				_radius = radius;
				_height = height;
				
				_rotation = glm::mat3(glm::eulerAngleXYZ(pitch, yaw, roll));

				_mat_index = mat_index;
			}
			catch (const std::exception& e) { throw; }
		}
		Cylinder(const glm::vec3& position, float radius, const int mat_index)
			: Object(position, mat_index), _radius(radius) {}

		float		getRadius() const { return (_radius); }
		float		getHeight() const { return (_height); }
		glm::mat3	getRotation() const { return (_rotation); }

		Type	getType() const override { return Type::CYLINDER; }

	private:
		float		_radius;
		float		_height;

		glm::mat3	_rotation;
};

#endif