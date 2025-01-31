/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BVH.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:48:48 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/31 19:30:54 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BVH.hpp"

BVH::BVH(std::vector<Triangle> &triangles, int first_primitive, int primitive_count) : _aabb(AABB(glm::vec3(1e30f), glm::vec3(-1e30f)))
{
	_left = nullptr;
	_right = nullptr;

	_is_leaf = true;
	
	_first_primitive = first_primitive;
	_primitive_count = primitive_count;
	
	updateBounds(triangles);
	subdivide(triangles);
}

void	BVH::updateBounds(std::vector<Triangle> &triangles)
{
	for (int i = 0; i < _primitive_count; i++)
	{
		Triangle leaf_triangle = triangles[_first_primitive + i];
		
		_aabb.min = glm::min(_aabb.min, leaf_triangle.getPosition());
        _aabb.min = glm::min(_aabb.min, leaf_triangle.getVertex2());
        _aabb.min = glm::min(_aabb.min, leaf_triangle.getVertex3());
        _aabb.max = glm::max(_aabb.max, leaf_triangle.getPosition());
        _aabb.max = glm::max(_aabb.max, leaf_triangle.getVertex2());
        _aabb.max = glm::max(_aabb.max, leaf_triangle.getVertex3());
	}
}


float	BVH::evaluateSah(std::vector<Triangle> &triangles, int axis, float pos)
{
    AABB left_box(glm::vec3(1e30f), glm::vec3(-1e30f));
	AABB right_box(glm::vec3(1e30f), glm::vec3(-1e30f));

    int left_count = 0;
	int right_count = 0;

    for (int i = 0; i < _primitive_count; i++)
    {
        Triangle triangle = triangles[_first_primitive + i];

        if (triangle.getCentroid()[axis] < pos)
        {
            left_count++;
            left_box.grow( triangle.getPosition() );
            left_box.grow( triangle.getVertex2() );
            left_box.grow( triangle.getVertex3() );
        }
        else
        {
            right_count++;
            right_box.grow( triangle.getPosition() );
            right_box.grow( triangle.getVertex2() );
            right_box.grow( triangle.getVertex3() );
        }
    }
    float cost = left_count * left_box.area() + right_count * right_box.area();
    return (cost > 0 ? cost : 1e30f);
}

void	BVH::subdivide(std::vector<Triangle> &triangles)
{
	if (_primitive_count <= 4)
		return ;

	const int num_test_per_axis = 5;

	int best_axis = 0;
	float best_pos = 0;
	float best_cost = 1e30f;

	for (int axis = 0; axis < 3; axis++)
	{
		float start_pos = _aabb.min[axis];
		float end_pos = _aabb.max[axis];

		for (int i = 0; i < num_test_per_axis; i++)
		{
			float split_t = (i + 1) / (num_test_per_axis + 1.0f);
			float candidate_pos = start_pos + (end_pos - start_pos) * split_t;

			float cost = evaluateSah(triangles, axis, candidate_pos);

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

	int i = _first_primitive;
	int j = _first_primitive + _primitive_count - 1;

	while (i <= j)
	{
		Triangle triangle = triangles[i];

		if (triangle.getCentroid()[axis] < split_pos)
			i++;
		else
		{
			std::swap(triangles[i], triangles[j]);
			j--;
		}
	}

	int left_count = i - _first_primitive;
	
	if (left_count == 0 || left_count == _primitive_count)
		return ;

	_is_leaf = false;

	_left = new BVH(triangles, _first_primitive, left_count);
	_right = new BVH(triangles, i , _primitive_count - left_count);

	_primitive_count = 0;
}

const AABB &BVH::getAABB() const
{
	return (_aabb);
}

GPUBvh	BVH::toGPUBvh()
{
	GPUBvh bvh;

	bvh.index = _first_primitive;
	bvh.primitive_count = _primitive_count;
	bvh.max = _aabb.max;
	bvh.min = _aabb.min;

	return (bvh); 
}

void BVH::flatten(std::vector<GPUBvh> &bvhs, int &currentIndex)
{
    GPUBvh self_bvh = toGPUBvh();
    int self_index = currentIndex++;

    if (!_is_leaf)
    {
        _left->flatten(bvhs, currentIndex);

        self_bvh.index = currentIndex;
        _right->flatten(bvhs, currentIndex);
    }

    bvhs[self_index] = self_bvh;
}

std::vector<GPUBvh>	BVH::getGPUBvhs()
{
	std::vector<GPUBvh> bvhs(getSize() + 1);
    
    int currentIndex = 0;
    flatten(bvhs, currentIndex);


    return (bvhs);
}

int	BVH::getSize()
{
	int count = 0;

	if (_is_leaf)
		return (0);
	
	count += 1 + _left->getSize();
	count += 1 + _right->getSize();

	return (count);
}

int	BVH::getLeaves()
{
	int count = 0;

	if (_is_leaf)
		return (1);
	
	count += _left->getLeaves();
	count += _right->getLeaves();

	return (count);
}

BVHStats BVH::analyzeBVHLeaves(BVH *root, int current_depth)
{
    if (!root)
        return {0, 0, 0.0f, INT_MAX, 0, 0.0f, 0};

    // If this is a leaf node, return its stats
    if (root->_is_leaf)
        return {
            root->_primitive_count, // min triangles
            root->_primitive_count, // max triangles
            (float)root->_primitive_count, // average triangles
            current_depth, // min depth
            current_depth, // max depth
            (float)current_depth, // depth sum (will be averaged later)
            1 // this leaf counts as 1 towards total leaves
        };

    // Get stats from left and right subtrees
    BVHStats left = analyzeBVHLeaves(root->_left, current_depth + 1);
    BVHStats right = analyzeBVHLeaves(root->_right, current_depth + 1);

    // Combine the results
    BVHStats combined;
    
    // Triangle statistics
    combined.min_triangles = std::min(left.min_triangles, right.min_triangles);
    combined.max_triangles = std::max(left.max_triangles, right.max_triangles);

    // Depth statistics
    combined.min_depth = std::min(left.min_depth, right.min_depth);
    combined.max_depth = std::max(left.max_depth, right.max_depth);
    
    // Count total leaves
    combined.total_leaves = left.total_leaves + right.total_leaves;

    // Calculate weighted averages
    if (combined.total_leaves > 0) {
        // Average triangles
        combined.average_triangles = 
            (left.average_triangles * left.total_leaves + 
             right.average_triangles * right.total_leaves) / combined.total_leaves;
        
        // Average depth
        combined.average_depth = 
            (left.average_depth * left.total_leaves + 
             right.average_depth * right.total_leaves) / combined.total_leaves;
    } else {
        combined.average_triangles = 0.0f;
        combined.average_depth = 0.0f;
    }

    return combined;
}