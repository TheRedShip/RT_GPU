/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BVH.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:36:19 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/24 19:35:58 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_BVH__HPP
# define RT_BVH__HPP

# include "RT.hpp"

struct GPUTriangle;
struct GPUObject;
struct GPUBvh;

struct AABB
{
	glm::vec3				min;
	glm::vec3				max;

	AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
	
	void grow( glm::vec3 p ) { min = glm::min( min, p ), max = glm::max( max, p ); }

    float area()
    { 
        glm::vec3 e = max - min;
        return (e.x * e.y + e.y * e.z + e.z * e.x); 
    }
};

struct BVHStats
{
    int min_triangles;
    int max_triangles;
    float average_triangles;
};

class BVH
{
	public:
		BVH(std::vector<Triangle> &triangles, int first_primitive, int primitive_count);


		void	updateBounds(std::vector<Triangle> &triangles);
		void	subdivide(std::vector<Triangle> &triangles);

		float	evaluateSah(std::vector<Triangle> &triangles, int axis, float pos);

		int							getSize();
		int							getLeaves();
		BVHStats					analyzeBVHLeaves(BVH* root);

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