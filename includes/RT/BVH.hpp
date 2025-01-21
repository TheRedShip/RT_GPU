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

struct GPUTriangle;
struct GPUObject;
struct GPUBvh;
struct AABB;

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