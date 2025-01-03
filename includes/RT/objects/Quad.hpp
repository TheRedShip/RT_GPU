/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quad.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:47:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_QUAD__HPP
# define RT_QUAD__HPP

# include "RT.hpp"

class Quad : public Object
{
	public:
		Quad(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z;
				float	x1, y1, z1;
				float	x2, y2, z2;
				int		mat_index;

				if (!(line >> x >> y >> z))
					throw std::runtime_error("Missing position");

				if (!(line >> x1 >> y1 >> z1))
					throw std::runtime_error("Missing quad's first edge ");

				if (!(line >> x2 >> y2 >> z2))
					throw std::runtime_error("Missing quad's second edge");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);
				_edge1 = glm::vec3(x1, y1, z1);
				_edge2 = glm::vec3(x2, y2, z2);

				_mat_index = mat_index;
			}
			catch (const std::exception &e) { throw; }
		}
		Quad(const glm::vec3 &position, const glm::vec3 &edge1, const glm::vec3 &edge2, const int mat_index)
			: Object(position, mat_index), _edge1(edge1), _edge2(edge2) {}

		glm::vec3	getEdge1() const { return (_edge1); }
		glm::vec3	getEdge2() const { return (_edge2); }
		Type		getType() const override { return Type::QUAD; }

	private:
		glm::vec3	_edge1;
		glm::vec3	_edge2;
};

#endif