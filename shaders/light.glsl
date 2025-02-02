hitInfo	traceRay(Ray ray);

vec3 GetEnvironmentLight(Ray ray)
{
    // return vec3(0.);
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

vec3 sampleSphereLight(vec3 position, GPUObject obj, int light_index, GPUMaterial mat, inout uint rng_state)
{
    float theta = 2.0 * M_PI * randomValue(rng_state);
    float phi = acos(2.0 * randomValue(rng_state) - 1.0);
    
    vec3 sample_point = obj.position + obj.radius * vec3(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    );
    
    vec3 light_dir = normalize(sample_point - position);
    float light_dist = length(sample_point - position);
    
    Ray shadow_ray = Ray(position + light_dir * 0.001, light_dir, (1.0 / light_dir));
    hitInfo shadow_hit = traceRay(shadow_ray);
    
    if (shadow_hit.obj_index != light_index)
        return vec3(0.0);

    float cos_theta = max(0.0, -dot(light_dir, normalize(sample_point - obj.position)));
    return mat.emission * mat.color / (light_dist * light_dist) * cos_theta / (4.0 * M_PI * (obj.radius / 2.0) * (obj.radius / 2.0));
}

vec3 sampleQuadLight(vec3 position, GPUObject obj, int light_index, GPUMaterial mat, inout uint rng_state)
{
    float u = randomValue(rng_state);
    float v = randomValue(rng_state);
    
    vec3 sample_point = obj.position + u * obj.vertex1 + v * obj.vertex2;
    vec3 light_dir = normalize(sample_point - position);
    float light_dist = length(sample_point - position);
    
    Ray shadow_ray = Ray(position + light_dir * 0.001, light_dir, (1.0 / light_dir));
    hitInfo shadow_hit = traceRay(shadow_ray);
    
    if (shadow_hit.obj_index != light_index)
        return vec3(0.0);

	vec3 crossQuad = cross(obj.vertex1, obj.vertex2);
    float area = length(crossQuad);
    float pdf = 1.0 / area;

    vec3 normal = normalize(crossQuad);
    float cos_theta = max(0.0, dot(normal, -light_dir));
    return mat.emission * mat.color / (light_dist);
}

vec3 sampleLights(vec3 position, inout uint rng_state)
{
	vec3 light = vec3(0.0);
    
    for (int i = 0; i < u_lightsNum; i++)
    {
        int light_index = lightsIndex[i];

        GPUObject obj = objects[light_index];
        GPUMaterial mat = materials[obj.mat_index];

        if (obj.type == 0)
            light += sampleSphereLight(position, obj, light_index, mat, rng_state);
        else if (obj.type == 2)
            light += sampleQuadLight(position, obj, light_index, mat, rng_state);
    }

	return (light);
}

vec2 getSphereUV(vec3 surfacePoint)
{
    float phi = atan(surfacePoint.z, surfacePoint.x);
    float theta = acos(surfacePoint.y);
    
    float u = (phi + M_PI) / (2.0 * M_PI);
    float v = theta / M_PI;
    
    return vec2(u, v);
}

uniform sampler2D textures[64];
uniform sampler2D emissive_textures[64];

vec2 getTextureColor(hitInfo hit)
{
    vec2 uv = vec2(0.0);

    if (hit.obj_type == 0)
        uv = getSphereUV(hit.normal);
    else if (hit.obj_type == 3)
    {
        GPUTriangle tri = triangles[hit.obj_index];
        uv = hit.u * tri.texture_vertex2 + hit.v * tri.texture_vertex3 + (1 - (hit.u + hit.v)) * tri.texture_vertex1;
    }
    return (vec2(uv.x, 1 - uv.y));
}

void    calculateLightColor(GPUMaterial mat, hitInfo hit, inout vec3 color, inout vec3 light, inout uint rng_state)
{
    if (mat.texture_index != -1)
    {
        vec2 uv = getTextureColor(hit);
        color *= texture(textures[mat.texture_index], uv).rgb;
    }
    if (mat.emission_texture_index != -1)
    {
        vec2 uv = getTextureColor(hit);
        vec3 emission = mat.emission * texture(emissive_textures[mat.emission_texture_index], uv).rgb;
       
        light += mat.emission * emission; 
    }
    else
    {
        color *= mat.color;
        light += mat.emission * mat.color;
    }
    // light += sampleLights(hit.position, rng_state);
}