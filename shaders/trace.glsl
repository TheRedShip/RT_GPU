

Ray	portalRay(Ray ray, hitInfo hit)
{
	GPUObject	portal_1;
	GPUObject	portal_2;
	vec3		relative;

	portal_1 = objects[hit.obj_index];
	portal_2 = objects[int(portal_1.radius)]; // saving memory radius = portal_index
		
	relative = hit.position - portal_1.position;
	
	mat3	rotation = mat3(portal_2.rotation) * transpose(mat3(portal_1.rotation));

	if (dot(portal_1.normal, portal_2.normal) > 0.0)
	{
		mat3 reflection = mat3(1.0) - 2.0 * outerProduct(portal_2.normal, portal_2.normal);
		rotation *= reflection;
	}

	ray.origin = portal_2.position + rotation * relative;
	ray.direction = normalize(rotation * ray.direction);

	ray.origin += ray.direction * 0.01f;

	return (ray);
}

hitInfo	traceScene(Ray ray)
{
	hitInfo hit;

	hit.t = 1e30;
	hit.obj_index = -1;
	
	for (int i = 0; i < u_objectsNum; i++)
	{
		GPUObject obj = objects[i];

		hitInfo temp_hit;
		if (intersect(ray, obj, temp_hit) && temp_hit.t < hit.t)
		{
			hit.t = temp_hit.t;
			hit.last_t = temp_hit.last_t;
			hit.obj_index = i;
			hit.mat_index = obj.mat_index;
			hit.position = temp_hit.position;
			hit.normal = temp_hit.normal;
		}
	}

	return (hit);
}

hitInfo	traceBVH(Ray ray)
{
	hitInfo temp_hit;
	hitInfo hit;

	hit.t = 1e30;
	hit.obj_index = -1;

    int stack[32];
    int stack_ptr = 0;
    stack[0] = 0;

    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];

        GPUBvh node = bvh[current_index];
        
		if (node.is_leaf != 0)
		{
			for (int i = 0; i < node.primitive_count; i++)
			{
				GPUTriangle obj = triangles[node.first_primitive + i];
				
				if (intersectTriangle(ray, obj, temp_hit) && temp_hit.t < hit.t)
				{
					hit.t = temp_hit.t;
					hit.last_t = temp_hit.last_t;
					hit.obj_index = node.first_primitive + i;
					hit.mat_index = obj.mat_index;
					hit.position = temp_hit.position;
					hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUBvh left_node = bvh[node.left_index];
			GPUBvh right_node = bvh[node.right_index];

			hitInfo left_hit;
			hitInfo right_hit;

			left_hit.t = 1e30;
			right_hit.t = 1e30;

			bool left_bool = intersectRayBVH(ray, left_node, left_hit);
			bool right_bool = intersectRayBVH(ray, right_node, right_hit);

			if (left_hit.t > right_hit.t)
			{
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
			}
			else
			{
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
			}
		}
    }
    
	return (hit);
}

hitInfo traceRay(Ray ray)
{
	hitInfo hitBVH;
	hitInfo hitScene;
	hitInfo hit;

	for (int i = 0; i < 10; i++) // portal ray
	{
		hitBVH = traceBVH(ray);
		hitScene = traceScene(ray);
		
		hit = hitBVH.t < hitScene.t ? hitBVH : hitScene;
		#if 0
			if (hit.obj_index == -1 || objects[hit.obj_index].type != 5)
				break ;
			ray = portalRay(ray, hit);
			ray.inv_direction = (1.0 / ray.direction);
		#else
			return (hit);
		#endif
	}

	return (hit);
}