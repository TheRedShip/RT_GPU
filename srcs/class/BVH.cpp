/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BVH.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 21:48:48 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/16 21:48:48 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BVH.hpp"

BVH::BVH(std::vector<GPUObject> primitives, int first_primitive, int primitive_count) : _aabb(AABB(glm::vec3(1e30f), glm::vec3(-1e30f)))
{
	_left = nullptr;
	_right = nullptr;

	_is_leaf = true;
	
	_first_primitive = first_primitive;
	_primitive_count = primitive_count;
	
	updateBounds(primitives);
	subdivide(primitives);
}

void	BVH::updateBounds(std::vector <GPUObject> primitives)
{
	for (int i = 0; i < _primitive_count; i++)
	{
		GPUObject leaf_triangle = primitives[_first_primitive + i];
		
		if (leaf_triangle.type != (int)Object::Type::TRIANGLE)
			continue ;

		if (leaf_triangle.type == (int)Object::Type::TRIANGLE)
		{
			leaf_triangle.vertex1 += leaf_triangle.position;
			leaf_triangle.vertex2 += leaf_triangle.position;
		}
		
		_aabb.min = glm::min(_aabb.min, leaf_triangle.position);
        _aabb.min = glm::min(_aabb.min, leaf_triangle.vertex1);
        _aabb.min = glm::min(_aabb.min, leaf_triangle.vertex2);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.position);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.vertex1);
        _aabb.max = glm::max(_aabb.max, leaf_triangle.vertex2);
	}
}

void	BVH::subdivide(std::vector<GPUObject> primitives)
{
	if (_primitive_count <= 2)
		return ;

	std::cout << "subdivide" << std::endl;

	glm::vec3 extent = _aabb.max - _aabb.min;
	
	int axis = 0; 
	if (extent.y > extent.x) axis = 1;
	if (extent.z > extent[axis]) axis = 2;

	glm::vec3 split_pos = _aabb.min + extent[axis] * 0.5f;

	int i = _first_primitive;
	int j = _first_primitive + _primitive_count - 1;

	while (i <= j)
	{
		glm::vec3 centroid = primitives[i].position + primitives[i].vertex1 + primitives[i].vertex2;
		centroid /= 3.0f;

		if (centroid[axis] < split_pos[axis])
			i++;
		else
		{
			std::swap(primitives[i], primitives[j]);
			j--;
		}
	}

	int left_count = i - _first_primitive;
	
	std::cout << "left bvh starts from " << _first_primitive << " and has " << left_count << " primitives" << std::endl;
	std::cout << "right bvh starts from " << i << " and has " << _primitive_count - left_count << " primitives" << std::endl;
	
	if (left_count == 0 || left_count == _primitive_count)
		return ;


	_is_leaf = false;

	_left = new BVH(primitives, _first_primitive, left_count);
	_right = new BVH(primitives, i, _primitive_count - left_count);

	_primitive_count = 0;
}

void	BVH::showAABB(Scene *scene)
{
	if (_is_leaf)
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
		_left->showAABB(scene);
		_right->showAABB(scene);
	}
}

const AABB &BVH::getAABB() const
{
	return (_aabb);
}