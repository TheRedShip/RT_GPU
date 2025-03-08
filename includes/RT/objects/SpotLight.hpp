/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SpotLight.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 18:50:04 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/08 18:50:04 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SPOTLIGHT__HPP
# define RT_SPOTLIGHT__HPP

# include "RT.hpp"

class SpotLight : public Object
{
	public:
		SpotLight(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			float	x, y, z, radius;
			float	dir_x, dir_y, dir_z;
			float	angle;

			int		mat_index;

			if (!(line >> x >> y >> z >> radius))
				throw std::runtime_error("Missing position or radius values");

			if (radius <= 0.0f)
				throw std::runtime_error("Radius must be positive");
			
			if (!(line >> dir_x >> dir_y >> dir_z))
				throw std::runtime_error("Missing direction values");
			
			if (!(line >> angle))
				throw std::runtime_error("Missing angle value");

			if (!(line >> mat_index))
				throw std::runtime_error("Missing material properties");

			_position = glm::vec3(x, y, z);
			_radius = radius / 2.0;

			_direction = glm::normalize(glm::vec3(dir_x, dir_y, dir_z));
			_angle = angle;
			
			_mat_index = mat_index;
		}
		SpotLight(const glm::vec3& position, float radius, const int mat_index)
			: Object(position, mat_index), _radius(radius) {}

		float			getRadius() const { return (_radius); }
		float			getAngle() const { return (_angle); }
		const glm::vec3	&getDirection() const { return (_direction); }

		Type	getType() const override { return Type::SPOTLIGHT; }

	private:
		float		_radius;

		glm::vec3	_direction;
		float		_angle;
};

#endif