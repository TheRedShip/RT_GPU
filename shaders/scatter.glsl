
Ray lambertRay(hitInfo hit, Ray ray, GPUMaterial mat, uint rng_state)
{
	vec3 diffuse_dir = normalize(hit.normal + randomDirection(rng_state));
	vec3 specular_dir = reflect(ray.direction, hit.normal);

	bool is_specular = (mat.metallic >= randomValue(rng_state));

	ray.origin = hit.position + hit.normal * 0.001;
	ray.direction = normalize(mix(diffuse_dir, specular_dir, mat.roughness * float(is_specular)));

	return (ray);
}

Ray dieletricRay(hitInfo hit, Ray ray, GPUMaterial mat)
{
    float	refraction_ratio;
	vec3	unit_direction;

    refraction_ratio = 1.0f / mat.roughness;  //mat.roughness = refraction (saving memory)

	if (dot(ray.direction, hit.normal) > 0.0f)
	{
		hit.normal = -hit.normal;
		refraction_ratio = mat.roughness;
	}

	unit_direction = normalize(ray.direction);
	ray.origin = hit.position + hit.normal * -0.0001f;
	ray.direction = refract(unit_direction, hit.normal, refraction_ratio);
	
    return (ray);
}


Ray newRay(hitInfo hit, Ray ray, uint rng_state)
{
    GPUObject	obj;
	GPUMaterial	mat;

	obj = objects[hit.obj_index];
	mat = materials[obj.mat_index];

    if (mat.type == 0)
        return (lambertRay(hit, ray, mat, rng_state));
    else if (mat.type == 1)
        return (dieletricRay(hit, ray, mat));
    return (ray);
}