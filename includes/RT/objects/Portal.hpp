/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Portal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:47:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_PORTAL__HPP
# define RT_PORTAL__HPP

# include "RT.hpp"

class Portal : public Object
{
	public:
		Portal(std::stringstream &line) : Object(glm::vec3(0.0f), -1)
		{
			try {
				float	x, y, z;
				float	x1, y1, z1;
				float	x2, y2, z2;
				int		linked_portal;
				int		mat_index;

				if (!(line >> x >> y >> z))
					throw std::runtime_error("Missing position");

				if (!(line >> x1 >> y1 >> z1))
					throw std::runtime_error("Missing Portal's first edge ");

				if (!(line >> x2 >> y2 >> z2))
					throw std::runtime_error("Missing Portal's second edge");
				
				if (!(line >> linked_portal))
					throw std::runtime_error("Missing Portal's linked index");

				if (!(line >> mat_index))
					throw std::runtime_error("Missing material properties");

				_position = glm::vec3(x, y, z);
				_edge1 = glm::vec3(x1, y1, z1);
				_edge2 = glm::vec3(x2, y2, z2);

				glm::vec3 up = glm::normalize(_edge1);
				glm::vec3 right = glm::normalize(_edge2);
				glm::vec3 forward = glm::normalize(glm::cross(right, up));

				up = normalize(glm::cross(forward, right));

				_transform = glm::mat3(right, up, forward);
				_normal = forward;

				std::cout << "Portal Transform Matrix:" << std::endl;
				std::cout << glm::to_string(_transform) << std::endl;

				_linked_portal = linked_portal;

				_mat_index = mat_index;
			}
			catch (const std::exception &e) { throw; }
		}
		Portal(const glm::vec3 &position, const glm::vec3 &edge1, const glm::vec3 &edge2, const int linked_portal, const int mat_index)
			: Object(position, mat_index), _edge1(edge1), _edge2(edge2), _linked_portal(linked_portal) {}

		glm::vec3	getEdge1() const { return (_edge1); }
		glm::vec3	getEdge2() const { return (_edge2); }
		glm::vec3	getNormal() const { return (_normal); }
		
		glm::mat3	getTransform() const { return (_transform); }
		
		int			getLinkedPortalIndex() const { return (_linked_portal); }

		Type		getType() const override { return Type::PORTAL; }

	private:
		glm::vec3	_edge1;
		glm::vec3	_edge2;
		glm::vec3	_normal;

		glm::mat3	_transform;

		int			_linked_portal;
};

#endif