/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sphere.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:47:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SPHERE__HPP
# define RT_SPHERE__HPP

# include "RT.hpp"

class Sphere : public Object
{
	public:
		Sphere(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z, radius;
				int		mat_index;

				if (!(line >> x >> y >> z >> radius))
					throw std::runtime_error("Missing position or radius values");

				if (radius <= 0.0f)
					throw std::runtime_error("Radius must be positive");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);
				_radius = radius;
				
				_mat_index = mat_index;
			}
			catch (const std::exception& e) { throw; }
		}
		Sphere(const glm::vec3& position, float radius, const int mat_index)
			: Object(position, mat_index), _radius(radius) {}

		float	getRadius() const { return (_radius); }
		Type	getType() const override { return Type::SPHERE; }

	private:
		float		_radius;
};

#endif