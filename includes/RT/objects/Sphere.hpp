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
		Sphere(const glm::vec3& position, float radius, const Material *material)
			: Object(position, material), _radius(radius) {}

		float getRadius() const { return (_radius); }
		Type getType() const override { return Type::SPHERE; }

	private:
		float	_radius;
};

#endif