/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TopBVH.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/21 21:46:45 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/22 19:32:43 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TopBVH.hpp"

TopBVH::TopBVH(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs, int first_bvh, int bvh_count) : _aabb(AABB(glm::vec3(1e30f), glm::vec3(-1e30f)))
{
	_left = nullptr;
	_right = nullptr;

	_is_leaf = true;
	
	_first_bvh = first_bvh;
	_bvh_count = bvh_count;
	
	updateBounds(bvhs_data, bvhs);
	subdivide(bvhs_data, bvhs);
}

void	TopBVH::updateBounds(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs)
{
	for (int i = 0; i < _bvh_count; i++)
    {
        GPUBvhData bvh_data = bvhs_data[_first_bvh + i];
        GPUBvh root_bvh = bvhs[bvh_data.bvh_start_index];

        glm::vec3 corners[8] = {
            glm::vec3(root_bvh.min.x, root_bvh.min.y, root_bvh.min.z),
            glm::vec3(root_bvh.max.x, root_bvh.min.y, root_bvh.min.z),
            glm::vec3(root_bvh.min.x, root_bvh.max.y, root_bvh.min.z),
            glm::vec3(root_bvh.max.x, root_bvh.max.y, root_bvh.min.z),
            glm::vec3(root_bvh.min.x, root_bvh.min.y, root_bvh.max.z),
            glm::vec3(root_bvh.max.x, root_bvh.min.y, root_bvh.max.z),
            glm::vec3(root_bvh.min.x, root_bvh.max.y, root_bvh.max.z),
            glm::vec3(root_bvh.max.x, root_bvh.max.y, root_bvh.max.z)
        };

        glm::vec3 transformed_min(1e30);
        glm::vec3 transformed_max(-1e30);

        for (glm::vec3 corner : corners)
        {
            glm::vec3 transformed = glm::vec3(glm::inverse(bvh_data.transform) * glm::vec4(corner, 1.0)) + bvh_data.offset;
            transformed_min = glm::min(transformed_min, transformed);
            transformed_max = glm::max(transformed_max, transformed);
        }

        _aabb.min = glm::min(_aabb.min, transformed_min);
        _aabb.max = glm::max(_aabb.max, transformed_max);
    }
}

float	TopBVH::evaluateSah(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs, int axis, float pos)
{
	AABB left_box(glm::vec3(1e30f), glm::vec3(-1e30f));
	AABB right_box(glm::vec3(1e30f), glm::vec3(-1e30f));

	int left_count = 0;
	int right_count = 0;

	for (int i = 0; i < _bvh_count; i++)
	{
		GPUBvhData bvh_data = bvhs_data[_first_bvh + i];
		GPUBvh bvh_root = bvhs[bvh_data.bvh_start_index];

		glm::vec3 min = bvh_root.min + bvh_data.offset;
		glm::vec3 max = bvh_root.max + bvh_data.offset;

		glm::vec3 centroid = (min + max) * 0.5f;

		if (centroid[axis] < pos)
		{
			left_count++;
			left_box.grow( min );
			left_box.grow( max );
		}
		else
		{
			right_count++;
			right_box.grow( min );
			right_box.grow( max );
		}
	}

	return (left_box.area() * left_count + right_box.area() * right_count);
}

void	TopBVH::subdivide(std::vector<GPUBvhData> &bvhs_data, std::vector<GPUBvh> &bvhs)
{
	if (_bvh_count <= 1)
		return ;


	const int num_test_per_axis = 5;

	int best_axis = 0;
	float best_pos = 0.0f;
	float best_cost = 1e30f;

	for (int axis = 0; axis < 3; axis++)
	{
		float start_pos = _aabb.min[axis];
		float end_pos = _aabb.max[axis];

		for (int i = 0; i < num_test_per_axis; i++)
		{
			float split_t = (i + 1) / (num_test_per_axis + 1.0f);
			float candidate_pos = start_pos + (end_pos - start_pos) * split_t;

			float cost = evaluateSah(bvhs_data, bvhs, axis, candidate_pos);

			if (cost < best_cost)
			{
				best_pos = candidate_pos;
				best_axis = axis;
				best_cost = cost;
			}
		}
	}

	int axis = best_axis;
	float split_pos = best_pos;

	int i = _first_bvh;
	int j = _first_bvh + _bvh_count - 1;

	while (i <= j)
	{
		GPUBvhData bvh_data = bvhs_data[i];
		GPUBvh root = bvhs[bvh_data.bvh_start_index];
		glm::vec3 min = root.min + bvh_data.offset;
		glm::vec3 max = root.max + bvh_data.offset;
		glm::vec3 centroid = (min + max) * 0.5f;

		if (centroid[axis] < split_pos)
			i++;
		else
		{
			std::swap(bvhs_data[i], bvhs_data[j]);
			j--;
		}
	}

	int left_count = i - _first_bvh;
	
	if (left_count == 0 || left_count == _bvh_count)
		return ;

	_is_leaf = false;

	_left = new TopBVH(bvhs_data, bvhs, _first_bvh, left_count);
	_right = new TopBVH(bvhs_data, bvhs, i, _bvh_count - left_count);

	_bvh_count = 0;
}

int	TopBVH::getSize()
{
	int count = 0;

	if (_is_leaf)
		return (0);
	
	count += 1 + _left->getSize();
	count += 1 + _right->getSize();

	return (count);
}

void TopBVH::flatten(std::vector<GPUTopBvh> &top_bvhs, int &currentIndex)
{
    GPUTopBvh self_top_bvh;

	self_top_bvh.is_leaf = _is_leaf;
	self_top_bvh.first_bvh_data = _first_bvh;
	self_top_bvh.bvh_data_count = _bvh_count;
	self_top_bvh.max = _aabb.max;
	self_top_bvh.min = _aabb.min;

    int self_index = currentIndex++;

    self_top_bvh.left_index = -1;
    self_top_bvh.right_index = -1;

    if (!_is_leaf)
    {
        self_top_bvh.left_index = currentIndex;
        _left->flatten(top_bvhs, currentIndex);

        self_top_bvh.right_index = currentIndex;
        _right->flatten(top_bvhs, currentIndex);
    }

    top_bvhs[self_index] = self_top_bvh;
}

std::vector<GPUTopBvh>	TopBVH::getGPUTopBvhs()
{
	std::vector<GPUTopBvh> bvhs(getSize() + 1);
    
    int currentIndex = 0;
    flatten(bvhs, currentIndex);

    return (bvhs);
}