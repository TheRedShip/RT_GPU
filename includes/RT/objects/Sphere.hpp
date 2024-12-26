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
		struct ParseError : public std::runtime_error
		{
			ParseError(const std::string& msg) : std::runtime_error("Sphere parse error: " + msg) {}
		};

		Sphere(std::stringstream &line) : Object(glm::vec3(0.0f), nullptr)
		{
			_mat = new Material;

			try {
				float x, y, z, radius;
				float r, g, b, rough, spec;

				if (!(line >> x >> y >> z >> radius))
					throw std::invalid_argument("Missing position or radius values");

				if (radius <= 0.0f)
					throw std::invalid_argument("Radius must be positive");

				if (!(line >> r >> g >> b >> rough >> spec))
					throw std::invalid_argument("Missing material properties");

				if (r < 0.0f || r > 255.0f || g < 0.0f || g > 255.0f || b < 0.0f || b > 255.0f)
					throw std::invalid_argument("Color values must be between 0 and 255");

				if (rough < 0.0f || rough > 1.0f)
					throw std::invalid_argument("Roughness must be between 0 and 1");

				if (spec < 0.0f || spec > 1.0f)
					throw std::invalid_argument("Specular must be between 0 and 1");

				_position = glm::vec3(x, y, z);
				_radius = radius;
				
				_mat->color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
				_mat->roughness = rough;
				_mat->specular = spec;

				_material = _mat;
			}
			catch (const std::exception& e) { throw; }
		}
		Sphere(const glm::vec3& position, float radius, const Material *material)
			: Object(position, material), _radius(radius) {}

		float	getRadius() const { return (_radius); }
		Type	getType() const override { return Type::SPHERE; }

	private:
		Material	*_mat;
		float		_radius;
};

#endif