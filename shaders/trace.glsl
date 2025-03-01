

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
		mat3 reflection = mat3(1.0) - 2.0 * outerProduct(portal_1.normal, portal_2.normal);
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
			hit.obj_index = i;
			hit.obj_type = obj.type;
			hit.mat_index = obj.mat_index;
			hit.position = temp_hit.position;
			hit.normal = temp_hit.normal;
			hit.u = temp_hit.u;
			hit.v = temp_hit.v;
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
				GPUTriangle obj = triangles[bvh_data.triangle_start_index + node.index + i];
				
				hitInfo temp_hit;
				if (intersectTriangle(ray, obj, temp_hit) && temp_hit.t < hit.t)
				{
					hit.u = temp_hit.u;
					hit.v = temp_hit.v;
					hit.t = temp_hit.t;
					hit.obj_index = bvh_data.triangle_start_index + node.index + i;
					// hit.mat_index = obj.mat_index;
					// hit.position = temp_hit.position;
					// hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUBvh left_node = Bvh[bvh_data.bvh_start_index + current_index + 1];
			GPUBvh right_node = Bvh[bvh_data.bvh_start_index + node.index];

			hitInfo left_hit;
			hitInfo right_hit;

			left_hit.t = 1e30;
			right_hit.t = 1e30;

			bool left_bool = intersectRayBVH(ray, left_node, left_hit);
			bool right_bool = intersectRayBVH(ray, right_node, right_hit);

			if (left_hit.t > right_hit.t)
			{
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = current_index + 1;
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.index;
			}
			else
			{
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.index;
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = current_index + 1;
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

		float transformed_t = temp_hit.t / bvh_data.scale;
		if (transformed_t < hit.t)
		{
			GPUTriangle triangle = triangles[temp_hit.obj_index];

			hit.u = temp_hit.u;
			hit.v = temp_hit.v;
			hit.t = transformed_t;
			hit.obj_index = temp_hit.obj_index;
			hit.mat_index = triangle.mat_index;
		
			vec3 position = transformedRay.origin + transformedRay.direction * temp_hit.t;
			hit.position = inverseTransformMatrix * position + bvh_data.offset;
			
			vec3 normal = normalize(triangle.normal_vertex1 * (1.0 - hit.u - hit.v) + triangle.normal_vertex2 * hit.u + triangle.normal_vertex3 * hit.v);
			vec3 directed_normal = normal * sign(-dot(transformedRay.direction, normal));
			hit.normal = normalize(inverseTransformMatrix * directed_normal);
		}
	}

	hit.obj_type = 3;

	return (hit);
}

hitInfo traceRay(inout Ray ray)
{
	hitInfo hitBVH;
	hitInfo hitScene;
	hitInfo hit;

	#if 1
	for (int i = 0; i < 10; i++) // portal ray
	{
		hitBVH = traverseBVHs(ray);
		hitScene = traceScene(ray);
		
		hit = hitBVH.t < hitScene.t ? hitBVH : hitScene;
		if (hit.obj_index == -1 || objects[hit.obj_index].type != 5)
			break ;
		ray = portalRay(ray, hit);
		ray.inv_direction = (1.0 / ray.direction);
	}
	#else 
		hitBVH = traverseBVHs(ray);
		hitScene = traceScene(ray);
		return (hitBVH.t < hitScene.t ? hitBVH : hitScene);
	#endif

	return (hit);
}
