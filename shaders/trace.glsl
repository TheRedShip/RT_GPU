

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

	for (int p = 0; p < 25; p++) //portals
	{
		hit.t = 1e30;
		hit.obj_index = -1;
		
		for (int i = 0; i < u_objectsNum; i++)
		{
			GPUObject obj = objects[i];

			hitInfo temp_hit;
			if (intersect(ray, obj, temp_hit) && temp_hit.t > 0.0f && temp_hit.t < hit.t + 0.0001)
			{
				hit.t = temp_hit.t;
				hit.last_t = temp_hit.last_t;
				hit.obj_index = i;
				hit.position = temp_hit.position;
				hit.normal = temp_hit.normal;
			}
		}
		if (hit.obj_index == -1 || objects[hit.obj_index].type != 5)
			break ;
		ray = portalRay(ray, hit);
	}

	return (hit);
}

hitInfo	traceBVH(Ray ray)
{
	hitInfo hit;

	hit.t = 1e30;
	hit.obj_index = -1;

    const int MAX_STACK_SIZE = 64;
    int stack[MAX_STACK_SIZE];
    int stack_ptr = 0;
    
    stack[0] = 0;
    
    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];

        GPUBvh node = bvh[current_index];
        
        if (intersectRayBVH(ray, node))
        {
            if (node.is_leaf != 0)
            {
                for (int i = 0; i < node.primitive_count; i++)
                {
                    GPUObject obj = objects[node.first_primitive + i];
                    
                    hitInfo temp_hit;
					if (intersect(ray, obj, temp_hit) && temp_hit.t > 0.0f && temp_hit.t < hit.t + 0.0001)
					{
						hit.t = temp_hit.t;
						hit.last_t = temp_hit.last_t;
						hit.obj_index = node.first_primitive + i;
						hit.position = temp_hit.position;
						hit.normal = temp_hit.normal;
					}
                }
            }
            
            if (node.is_leaf == 0 && stack_ptr < MAX_STACK_SIZE - 2)
            {
                stack_ptr++;
                stack[stack_ptr] = node.left_index;
                stack_ptr++;
                stack[stack_ptr] = node.right_index;
            }
        }
    }
    
	return (hit);
}

hitInfo traceRay(Ray ray)
{
	if (true)
		return (traceBVH(ray));
	return (traceScene(ray));
}