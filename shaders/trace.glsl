

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

hitInfo traceBVH(Ray ray, GPUBvhData bvh_data)
{
	hitInfo hit;
	hitInfo hit_bvh;

	hit.t = 1e30;
	hit.obj_index = -1;

    int stack[32];
    int stack_ptr = 0;
    stack[0] = 0;
    
    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];
        GPUBvh node = Bvh[bvh_data.bvh_start_index + current_index];

		if (node.primitive_count != 0)
		{
			for (int i = 0; i < node.primitive_count; i++)
			{
				GPUTriangle obj = triangles[bvh_data.triangle_start_index + node.first_primitive + i];
				
				hitInfo temp_hit;
				if (intersectTriangle(ray, obj, temp_hit) && temp_hit.t < hit.t)
				{
					hit.t = temp_hit.t;
					hit.last_t = temp_hit.last_t;
					hit.obj_index = bvh_data.triangle_start_index + node.first_primitive + i;
					hit.mat_index = obj.mat_index;
					hit.position = temp_hit.position;
					hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUBvh left_node = Bvh[bvh_data.bvh_start_index + node.left_index];
			GPUBvh right_node = Bvh[bvh_data.bvh_start_index + node.right_index];

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

hitInfo traverseBVHs(Ray ray)
{
	hitInfo hit;
	
	hit.t = 1e30;
	hit.obj_index = -1;

	for (int i = 0; i < u_bvhNum; i++)
	{
		GPUBvhData bvh_data = BvhData[i];
		
		mat3 transformMatrix = mat3(bvh_data.transform);
		mat3 inverseTransformMatrix = mat3(bvh_data.inv_transform);

		Ray transformedRay;
		transformedRay.direction = normalize(transformMatrix * ray.direction);
		transformedRay.origin = transformMatrix * (ray.origin - bvh_data.offset);
		transformedRay.inv_direction = (1. / transformedRay.direction);
		
		hitInfo temp_hit = traceBVH(transformedRay, BvhData[i]);

		temp_hit.t = temp_hit.t / bvh_data.scale;

		if (temp_hit.t < hit.t)
		{
			hit.t = temp_hit.t;
			hit.last_t = temp_hit.last_t / bvh_data.scale;
			hit.obj_index = temp_hit.obj_index;
			hit.mat_index = temp_hit.mat_index;
			hit.position = inverseTransformMatrix * temp_hit.position + bvh_data.offset;
			hit.normal = normalize(inverseTransformMatrix * temp_hit.normal);
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
		hitBVH = traverseBVHs(ray);
		hitScene = traceScene(ray);
		
		hit = hitBVH.t < hitScene.t ? hitBVH : hitScene;
		#if 1
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