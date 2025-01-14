
vec3 GetEnvironmentLight(Ray ray)
{
	vec3 sun_pos = vec3(-0.5, 0.5, 0.5);
	float SunFocus = 1.5;
	float SunIntensity = 1.;

	vec3 GroundColour = vec3(0.5, 0.5, 0.5);
	vec3 SkyColourHorizon = vec3(135 / 255.0f, 206 / 255.0f, 235 / 255.0f);
	vec3 SkyColourZenith = SkyColourHorizon / 2.0;

	float skyGradientT = pow(smoothstep(0.0, 0.4, ray.direction.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0.0, ray.direction.y);
	vec3 skyGradient = mix(SkyColourHorizon, SkyColourZenith, skyGradientT);
	float sun = pow(max(0, dot(ray.direction, sun_pos.xyz)), SunFocus) * SunIntensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(GroundColour, skyGradient, groundToSkyT) + sun * int(groundToSkyT >= 1);
	return composite;
}

hitInfo	traceRay(Ray ray);

vec3	sampleLights(vec3 position)
{
	vec3 light = vec3(0.0);

	for (int i = 0; i < u_objectsNum; i++)
	{
		GPUObject obj = objects[i];
		GPUMaterial mat = materials[obj.mat_index];

		if (mat.emission > 0.0)
		{
			vec3 light_dir = normalize(obj.position - position);
			float light_dist = length(obj.position - position);

			Ray shadow_ray = Ray(position + light_dir * 0.01, light_dir);
			hitInfo shadow_hit = traceRay(shadow_ray);

			if (shadow_hit.obj_index == i)
				light += mat.emission * mat.color / (light_dist);
		}
	}

	return (light);
}

void    calculateLightColor(inout vec3 color, inout vec3 light, GPUMaterial mat, hitInfo hit)
{
    color *= mat.color;
    light += mat.emission * mat.color;
    // light += sampleLights(hit.position);
}