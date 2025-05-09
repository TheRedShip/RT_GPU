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
	float		emission;
	float		roughness;
	float		metallic;
	float		refraction;
	int			type;
	int			texture_index;
	int			emission_texture_index;
}				Material;

class Object
{
	protected:
		glm::vec3		_position;
		int				_mat_index;

	public:
		Object(const glm::vec3& position, const int mat_index) : _position(position), _mat_index(mat_index) {}
		virtual ~Object() = default;

		int		getMaterialIndex() const {return (_mat_index); }
		const glm::vec3	&getPosition() const { return (_position); }
	
		enum class Type {
        	SPHERE,
			PLANE,
			QUAD,
			TRIANGLE,
			CUBE,
			PORTAL,
			CYLINDER,
			SPOTLIGHT,
			LASERBEAM
    	};

    	virtual Type	getType() const = 0;
};

#endif