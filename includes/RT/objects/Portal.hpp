/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Portal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 19:12:51 by ycontre           #+#    #+#             */
/*   Updated: 2025/02/06 19:46:12 by ycontre          ###   ########.fr       */
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
			float	x, y, z;
			float	x1, y1, z1;
			float	x2, y2, z2;
			
			int		mat_index;

			if (!(line >> x >> y >> z))
				throw std::runtime_error("Missing position");

			if (!(line >> x1 >> y1 >> z1))
				throw std::runtime_error("Missing Portal's first edge ");

			if (!(line >> x2 >> y2 >> z2))
				throw std::runtime_error("Missing Portal's second edge");
			
			if (!(line >> _invert_normal))
				throw std::runtime_error("Missing invert_normal");

			if (!(line >> mat_index))
				throw std::runtime_error("Missing material properties");

			_position = glm::vec3(x, y, z);
			_up = glm::vec3(x1, y1, z1);
			_right = glm::vec3(x2, y2, z2);

			glm::vec3 right = glm::normalize(_right);
			glm::vec3 up = glm::normalize(_up);
			glm::vec3 forward = glm::normalize(glm::cross(right, up));

			_rotation = glm::mat3(right, up, forward);
			_normal = forward * (_invert_normal ? -1.0f : 1.0f);

			_linked_portal = -1;

			_mat_index = mat_index;
		}
		Portal(const glm::vec3 &position, const glm::vec3 &edge1, const glm::vec3 &edge2, const int linked_portal, const int mat_index)
			: Object(position, mat_index), _up(edge1), _right(edge2), _linked_portal(linked_portal) {}

		Quad		*createSupportQuad() const
		{
			float extension = 0.2f;
			
			glm::vec3 right_dir = glm::normalize(_right);
			glm::vec3 up_dir = glm::normalize(_up);

			float right_length = glm::length(_right) + extension;
			float up_length = glm::length(_up) + extension;

			glm::vec3 center_offset = -(right_dir * (extension / 2.0f) + up_dir * (extension / 2.0f));
			glm::vec3 position = _position + _normal * 0.001f + center_offset;

			glm::vec3 right = right_dir * right_length;
			glm::vec3 up = up_dir * up_length;

			return (new Quad(position, right, up, _normal, 1, _mat_index));
		}

		glm::vec3	getUp() const { return (_up); }
		glm::vec3	getRight() const { return (_right); }
		glm::vec3	getNormal() const { return (_normal); }
		
		glm::mat3	getRotation() const { return (_rotation); }
		
		int			getLinkedPortalIndex() const { return (_linked_portal); }
		void		setLinkedPortalIndex(int index) { _linked_portal = index; }

		Type		getType() const override { return Type::PORTAL; }

	private:
		glm::vec3	_up;
		glm::vec3	_right;
		glm::vec3	_normal;

		glm::mat3	_rotation;

		int			_linked_portal;
		int			_invert_normal;
};

#endif