/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Object.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:44:18 by ycontre           #+#    #+#             */
/*   Updated: 2024/12/23 19:12:39 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_OBJECT__HPP
# define RT_OBJECT__HPP

# include "RT.hpp"

typedef struct s_Material
{
	glm::vec3	color;
	float		roughness;
	float		specular;
}				Material;

class Object
{
	protected:
		glm::vec3		_position;
		const Material	*_material;

	public:
		Object(const glm::vec3& position, const Material *material) : _position(position), _material(material) {}
		virtual ~Object() = default;

		const glm::vec3	&getPosition() const { return (_position); }
		const Material	*getMaterial() const { return (_material); }
	
		enum class Type {
        	SPHERE,
    	};

    	virtual Type getType() const = 0;
};

#endif