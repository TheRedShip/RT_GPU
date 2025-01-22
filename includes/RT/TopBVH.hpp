/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TopBVH.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:36:19 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/17 18:59:28 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_TOPBVH__HPP
# define RT_TOPBVH__HPP

# include "RT.hpp"

struct GPUBvhData;
struct GPUTopBvh;
struct AABB;

class TopBVH
{
	public:
		TopBVH(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs, int first_bvh, int bvh_count);

		void	updateBounds(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs);
		void	subdivide(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs);

		float	evaluateSah(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs, int axis, float pos);

		int							getSize();
		// int							getLeaves();

		// void						flatten(std::vector<GPUTopBvh> &TopBVHs, int &currentIndex);
		// GPUTopBvh					toGPUTopBvh();
		std::vector<GPUTopBvh>		getGPUTopBvhs();
		void						flatten(std::vector<GPUTopBvh> &top_bvhs, int &currentIndex);

		// const AABB					&getAABB() const;
		// std::vector<GPUTopBvh>		getGPUTopBvhs();
		

	private:
		AABB					_aabb;
		
		TopBVH					*_left;
		TopBVH					*_right;

		bool					_is_leaf;

		int						_first_bvh;
		int						_bvh_count;
};

#endif