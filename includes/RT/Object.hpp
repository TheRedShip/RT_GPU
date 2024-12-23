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

class Object
{
	public:
		glm::vec3 position;
		glm::vec3 color;   
	
		Object(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
		virtual ~Object() = default;
	
		// virtual bool hit(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& t, glm::vec3& hitNormal) const = 0;

		
};

#endif