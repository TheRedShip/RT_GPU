/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BVH.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:36:19 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/17 18:59:28 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_BVH__HPP
# define RT_BVH__HPP

# include "RT.hpp"

struct GPUObject;
struct GPUBvh;

struct AABB
{
	glm::vec3				min;
	glm::vec3				max;

	AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
};


class BVH
{
	public:
		BVH(std::vector<GPUObject> &primitives, int first_primitive, int primitive_count);


		void	showAABB(Scene *scene);
		
		void	updateBounds(std::vector <GPUObject> &primitives);
		void	subdivide(std::vector<GPUObject> &primitives);

		int							size();

		void						flatten(std::vector<GPUBvh> &bvhs, int &currentIndex);
		GPUBvh						toGPUBvh();

		const AABB					&getAABB() const;
		std::vector<GPUBvh>			getGPUBvhs();
		

	private:
		AABB					_aabb;
		
		BVH						*_left;
		BVH						*_right;

		bool					_is_leaf;

		int						_first_primitive;
		int						_primitive_count;
};

#endif