
Ray		newRay(hitInfo hit, Ray ray, inout uint rng_state)
{
	GPUObject	obj;
	Ray			new_ray;

	obj = objects[hit.obj_index];
	
	vec3 diffuse_dir = normalize(hit.normal + randomDirection(rng_state));
	vec3 specular_dir = reflect(ray.direction, hit.normal);

	bool is_specular = (obj.metallic >= randomValue(rng_state));

	new_ray.origin = hit.position + hit.normal * 0.001;
	new_ray.direction = mix(diffuse_dir, specular_dir, obj.roughness * float(is_specular));

	return (new_ray);
}