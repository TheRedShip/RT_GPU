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

int	BVH::size()
{
	int count = 0;

	if (_is_leaf)
		return (0);
	
	count += 1 + _left->size();
	count += 1 + _right->size();

	return (count);
}

std::vector<GPUBvh>	BVH::getGPUBvhs()
{
	std::vector<GPUBvh> bvhs(size() + 1);
    
    int currentIndex = 0;
    flatten(bvhs, currentIndex);


    return (bvhs);
}