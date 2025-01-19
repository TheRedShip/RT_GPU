/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/17 18:17:30 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_TRIANGLE__HPP
# define RT_TRIANGLE__HPP

# include "RT.hpp"

class Triangle : public Object
{
	public:
		Triangle(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			float	x, y, z;
			float	x2, y2, z2;
			float	x3, y3, z3;

			int		mat_index;

			if (!(line >> x >> y >> z))
				throw std::runtime_error("Missing first vertex position");

			if (!(line >> x2 >> y2 >> z2))
				throw std::runtime_error("Missing second vertex position");
			
			if (!(line >> x3 >> y3 >> z3))
				throw std::runtime_error("Missing third vertex position");

			if (!(line >> mat_index))
				throw std::runtime_error("Missing material properties");

			_position = glm::vec3(x, y, z);
			_vertex2 = glm::vec3(x2, y2, z2);
			_vertex3 = glm::vec3(x3, y3, z3);
			
			// _vertex2 -= _position; //optimization
			// _vertex3 -= _position; //optimization

			_normal = glm::normalize(glm::cross(_vertex2 - _position, _vertex3 - _position)); //optimization
			_centroid = (_position + _vertex2 + _vertex3) / 3.0f;
		
			_mat_index = mat_index;
		}
		Triangle(const glm::vec3& position, const glm::vec3& vertex2, const glm::vec3& vertex3, const int mat_index)
			: Object(position, mat_index), _vertex2(vertex2), _vertex3(vertex3) {
				// _vertex2 -= _position; //optimization
				// _vertex3 -= _position; //optimization
		
				_normal = glm::normalize(glm::cross(_vertex2 - _position, _vertex3 - _position)); //optimization
				_centroid = (_position + _vertex2 + _vertex3) / 3.0f;
			}

		const glm::vec3		&getVertex2() const { return (_vertex2); }
		const glm::vec3		&getVertex3() const { return (_vertex3); }
		const glm::vec3		&getNormal() const { return (_normal); }
		const glm::vec3		&getCentroid() const { return (_centroid); }


		Type				getType() const override { return Type::TRIANGLE; }

	private:
		glm::vec3	_vertex2;
		glm::vec3	_vertex3;

		glm::vec3	_normal;

		glm::vec3	_centroid;
};

#endif
