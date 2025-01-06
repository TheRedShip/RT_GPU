/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cube.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:47:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_CUBE__HPP
# define RT_CUBE__HPP

# include "RT.hpp"

class Cube : public Object
{
	public:
		Cube(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z;
				float	width, height, depth;
				int		mat_index;

				if (!(line >> x >> y >> z))
					throw std::runtime_error("Missing position values");

				if (!(line >> width >> height >> depth))
					throw std::runtime_error("Missing width, height or depth values");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);
				_size = glm::vec3(width, height, depth);
				
				_mat_index = mat_index;
			}
			catch (const std::exception& e) { throw; }
		}
		Cube(const glm::vec3& position, const glm::vec3 &size, const int mat_index)
			: Object(position, mat_index), _size(size) {}

		const glm::vec3	&getSize() const { return (_size); }
		Type			getType() const override { return Type::CUBE; }

	private:
		glm::vec3	_size;

};

#endif