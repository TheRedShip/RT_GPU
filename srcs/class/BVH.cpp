/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BVH.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:48:48 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/18 21:11:35 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BVH.hpp"

BVH::BVH(std::vector<GPUTriangle> &primitives, int first_primitive, int primitive_count) : _aabb(AABB(glm::vec3(1e30f), glm::vec3(-1e30f)))
{
	_left = nullptr;
	_right = nullptr;

	_is_leaf = true;
	
	_first_primitive = first_primitive;
	_primitive_count = primitive_count;
	
	updateBounds(primitives);
	subdivide(primitives);
}

void	BVH::updateBounds(std::vector<GPUTriangle> &primitives)
{
	for (int i = 0; i < _primitive_count; i++)
	{
		GPUTriangle leaf_triangle = primitives[_first_primitive + i];
		
		// if (leaf_triangle.type != (int)Object::Type::TRIANGLE)
			// continue ;
		
		_aabb.min = glm::min(_aabb.min, leaf_triangle.position);
        _aabb.min = glm::min(_aabb.min, leaf_triangle.vertex1);
        _aabb.min = glm::min(_aabb.min, leaf_triangle.vertex2);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.position);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.vertex1);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.vertex2);
	}
}

void	BVH::subdivide(std::vector<GPUTriangle> &primitives)
{
	if (_primitive_count <= 4)
		return ;

	glm::vec3 extent = _aabb.max - _aabb.min;
	
	int axis = 0; 
	if (extent.y > extent.x) axis = 1;
	if (extent.z > extent[axis]) axis = 2;

	float split_pos = _aabb.min[axis] + extent[axis] * 0.5f;

	int i = _first_primitive;
	int j = _first_primitive + _primitive_count - 1;

	while (i <= j)
	{
		glm::vec3 centroid = primitives[i].position + primitives[i].vertex1 + primitives[i].vertex2;
		centroid /= 3.0f;

		if (centroid[axis] < split_pos)
			i++;
		else
		{
			std::swap(primitives[i], primitives[j]);
			j--;
		}
	}

	int left_count = i - _first_primitive;
	
	if (left_count == 0 || left_count == _primitive_count)
		return ;

	_is_leaf = false;

	_left = new BVH(primitives, _first_primitive, left_count);
	_right = new BVH(primitives, i , _primitive_count - left_count);

	_primitive_count = 0;
}

void	BVH::showAABB(Scene *scene)
{
	if (!_is_leaf)
	{
		scene->addObject(new Sphere(_aabb.min, 0.5f, 6));
		scene->addObject(new Sphere(_aabb.max, 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.min.x, _aabb.min.y, _aabb.max.z), 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.min.x, _aabb.max.y, _aabb.min.z), 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.max.x, _aabb.min.y, _aabb.min.z), 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.min.x, _aabb.max.y, _aabb.max.z), 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.max.x, _aabb.min.y, _aabb.max.z), 0.5f, 6));
		scene->addObject(new Sphere(glm::vec3(_aabb.max.x, _aabb.max.y, _aabb.min.z), 0.5f, 6));
	}
	else
	{
		// _left->showAABB(scene);
		// _right->showAABB(scene);
	}
}

const AABB &BVH::getAABB() const
{
	return (_aabb);
}

GPUBvh	BVH::toGPUBvh()
{
	GPUBvh bvh;

	bvh.is_leaf = _is_leaf;
	bvh.first_primitive = _first_primitive;
	bvh.primitive_count = _primitive_count;
	bvh.max = _aabb.max;
	bvh.min = _aabb.min;

	return (bvh); 
}

void BVH::flatten(std::vector<GPUBvh> &bvhs, int &currentIndex)
{
    GPUBvh self_bvh = toGPUBvh();
    int self_index = currentIndex++;

    self_bvh.left_index = -1;
    self_bvh.right_index = -1;

    if (!_is_leaf)
    {
        self_bvh.left_index = currentIndex;
        _left->flatten(bvhs, currentIndex);

        self_bvh.right_index = currentIndex;
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

//get tri per leaf min max and average
BVHStats	BVH::analyzeBVHLeaves(BVH *root)
{
    if (!root)
        return {0, 0, 0.0f};
    // If this is a leaf node, return its stats

    if (root->_is_leaf)
        return {
            root->_primitive_count,  // min
            root->_primitive_count,  // max
            (float)root->_primitive_count  // average
        };

    // Get stats from left and right subtrees
    BVHStats left = analyzeBVHLeaves(root->_left);
    BVHStats right = analyzeBVHLeaves(root->_right);

    // Combine the results
    int min_count = std::min(left.min_triangles, right.min_triangles);
    int max_count = std::max(left.max_triangles, right.max_triangles);
    
    // Calculate weighted average based on number of leaves in each subtree
    float left_leaf_count = (left.average_triangles > 0) ? 1.0f : 0.0f;
    float right_leaf_count = (right.average_triangles > 0) ? 1.0f : 0.0f;
    float total_leaf_count = left_leaf_count + right_leaf_count;
    
    float avg_count = 0.0f;
    if (total_leaf_count > 0)
        avg_count = (left.average_triangles * left_leaf_count + 
                    right.average_triangles * right_leaf_count) / total_leaf_count;

    return {min_count, max_count, avg_count};
}