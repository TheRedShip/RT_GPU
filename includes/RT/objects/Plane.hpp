/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Plane.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:47:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_Plane__HPP
# define RT_Plane__HPP

# include "RT.hpp"

class Plane : public Object
{
	public:
		Plane(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z;
				float	nx, ny, nz;
				int		mat_index;

				if (!(line >> x >> y >> z))
					throw std::runtime_error("Missing position");

				if (!(line >> nx >> ny >> nz))
					throw std::runtime_error("Missing plane's normal");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);
				_normal = glm::vec3(nx, ny, nz);
				
				_mat_index = mat_index;
			}
			catch (const std::exception &e) { throw; }
		}
		Plane(const glm::vec3 &position, const glm::vec3 &normal, const int mat_index)
			: Object(position, mat_index), _normal(normal) {}

		glm::vec3	getNormal() const { return (_normal); }
		Type		getType() const override { return Type::PLANE; }

	private:
		glm::vec3	_normal;
};

#endif