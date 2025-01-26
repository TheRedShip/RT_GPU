
Ray lambertRay(hitInfo hit, Ray ray, GPUMaterial mat, inout uint rng_state)
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

    refraction_ratio = 1.0f / mat.refraction;

	if (dot(ray.direction, hit.normal) > 0.0f)
	{
		hit.normal = -hit.normal;
		refraction_ratio = mat.refraction;
	}

	unit_direction = normalize(ray.direction);
	ray.origin = hit.position + hit.normal * -0.0001f;
	ray.direction = refract(unit_direction, hit.normal, refraction_ratio);
	
    return (ray);
}

void swap(inout float a, inout float b)
{
	float temp = a;
	a = b;
	b = temp;
}

float fresnel(vec3 incident, vec3 normal, float eta)
{
    float cosi = clamp(dot(incident, normal), -1.0, 1.0);
    float etai = 1.0, etat = eta;
    if (cosi > 0.0) swap(etai, etat);
    float sint = etai / etat * sqrt(max(0.0, 1.0 - cosi * cosi));
    if (sint >= 1.0)
        return (1.0); // Total internal reflection
    float cost = sqrt(max(0.0, 1.0 - sint * sint));
    cosi = abs(cosi);
    float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
    return (Rs * Rs + Rp * Rp) * 0.5;
}

Ray transparencyRay(hitInfo hit, Ray ray, GPUMaterial mat, inout uint rng_state)
{
    Ray newRay;
    
    float eta = mat.refraction;
    vec3 refractedDir = refract(ray.direction, hit.normal, 1.0 / eta);

    float kr = fresnel(ray.direction, hit.normal, eta);

    float randVal = randomValue(rng_state);
    if (randVal < mat.metallic || length(refractedDir) == 0.0)
	{
        newRay.origin = hit.position + hit.normal * 1e-4;
        newRay.direction = reflect(ray.direction, hit.normal);
    }
	else
	{
        newRay.origin = hit.position - hit.normal * 1e-4;
        newRay.direction = refractedDir;
    }

    return newRay;
}

Ray newRay(hitInfo hit, Ray ray, inout uint rng_state)
{
    GPUObject	obj;
	GPUMaterial	mat;

	obj = objects[hit.obj_index];
	mat = materials[hit.mat_index];

    if (mat.type == 0)
        return (lambertRay(hit, ray, mat, rng_state));
    else if (mat.type == 1)
        return (dieletricRay(hit, ray, mat));
	else if (mat.type == 2)
		return (transparencyRay(hit, ray, mat, rng_state));
    return (ray);
}
