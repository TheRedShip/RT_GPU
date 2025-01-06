
Ray		newRay(hitInfo hit, Ray ray, inout uint rng_state)
{
	GPUObject	obj;
	GPUMaterial	mat;
	Ray			new_ray;

	obj = objects[hit.obj_index];
	mat = materials[obj.mat_index];

	vec3 diffuse_dir = normalize(hit.normal + randomDirection(rng_state));
	vec3 specular_dir = reflect(ray.direction, hit.normal);

	bool is_specular = (mat.metallic >= randomValue(rng_state));

	new_ray.origin = hit.position + hit.normal * 0.001;
	new_ray.direction = mix(diffuse_dir, specular_dir, mat.roughness * float(is_specular));

	return (new_ray);
}