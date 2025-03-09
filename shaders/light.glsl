hitInfo	traceRay(inout Ray ray);

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

#define FOG_BOUNCE 0

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

    float beam_cos_angle = obj.vertex1.x;
    vec3 dir = obj.normal;
    
    if (shadow_hit.obj_index != light_index || dot(shadow_ray.direction, dir) < beam_cos_angle)
    {
        #if FOG_BOUNCE
            float circleRadius = light_dist * tan(acos(beam_cos_angle));

            float r = circleRadius * sqrt(randomValue(rng_state));
            float theta = 2.0 * M_PI * randomValue(rng_state);
            vec2 diskSample = vec2(r * cos(theta), r * sin(theta));

            vec3 up = abs(dir.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0);
            vec3 tangent = normalize(cross(up, dir));
            vec3 bitangent = cross(dir, tangent);

            vec3 circleCenter = obj.position + light_dist * dir;

            vec3 sample_point = circleCenter + diskSample.x * tangent + diskSample.y * bitangent;

            Ray light_ray = Ray(sample_point, -dir, (1.0 / -dir));
            hitInfo light_ray_hit = traceRay(light_ray);

            if (light_ray_hit.obj_index == -1)
                return (vec3(0.0));

            GPUMaterial light_ray_mat = materials[light_ray_hit.mat_index];
            if (light_ray_mat.metallic == 0.)
                return vec3(0.0);

            Ray reflect_ray = newRay(light_ray_hit, light_ray, rng_state);
            reflect_ray.inv_direction = 1.0 / reflect_ray.direction;

            vec3 reflect_to_particle = normalize(position - reflect_ray.origin);
            
            if (dot(reflect_ray.direction, reflect_to_particle) < beam_cos_angle)
                return vec3(0.0);

            mat.color *= light_ray_mat.color;
        #else
            return vec3(0.);
        #endif
    }

    float cos_theta = max(0.0, -dot(light_dir, normalize(sample_point - obj.position)));
    return mat.emission * mat.color / max(light_dist * light_dist, 1.0) * cos_theta / (4.0 * M_PI * (obj.radius / 2.0) * (obj.radius / 2.0));
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

    vec3 quad_to_camera = position - obj.position;
    float distance_plane = dot(quad_to_camera, obj.normal);
    vec3 point_projected = position - distance_plane * obj.normal;

    mat2 A = mat2(
        dot(obj.vertex1, obj.vertex1), dot(obj.vertex1, obj.vertex2),
        dot(obj.vertex1, obj.vertex2), dot(obj.vertex2, obj.vertex2)
    );
    vec2 b = vec2(
        dot(point_projected - obj.position, obj.vertex1),
        dot(point_projected - obj.position, obj.vertex2)
    );
    vec2 alphaBeta = inverse(A) * b;

    if (alphaBeta.x < 0.0f || alphaBeta.x > 1.0f || alphaBeta.y < 0.0f || alphaBeta.y > 1.0f)
        return vec3(0.0);

	vec3 crossQuad = cross(obj.vertex1, obj.vertex2);
    float area = length(crossQuad);
    float pdf = 1.0 / area;

    // float cos_theta = max(0.0, dot(obj.normal, -light_dir));
    return mat.emission * mat.color / max(light_dist * light_dist, 1.);
}

vec3 sampleLights(in vec3 position, inout uint rng_state)
{
    int light_list_index = int(floor(randomValue(rng_state) * float(u_lightsNum)));
    int light_index = lightsIndex[light_list_index];

    GPUObject light_obj = objects[light_index];
    GPUMaterial lightMat = materials[light_obj.mat_index];
    
    if (light_obj.type == 7)
        return float(u_lightsNum) * sampleSphereLight(position, light_obj, light_index, lightMat, rng_state);
    else if (light_obj.type == 2)
        return float(u_lightsNum) * sampleQuadLight(position, light_obj, light_index, lightMat, rng_state);
    return (vec3(0.));
}

vec2 getSphereUV(vec3 surfacePoint)
{
    float phi = atan(surfacePoint.z, surfacePoint.x);
    float theta = acos(surfacePoint.y);
    
    float u = (phi + M_PI) / (2.0 * M_PI);
    float v = theta / M_PI;
    
    return vec2(u, v);
}

#if SHADER_TEXTURE_MAX
    uniform sampler2D textures[SHADER_TEXTURE_MAX];
    uniform sampler2D emissive_textures[SHADER_TEXTURE_MAX];
#else
    uniform sampler2D textures[64];
    uniform sampler2D emissive_textures[64];
#endif

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

vec3    getCheckerboardColor(GPUMaterial mat, hitInfo hit)
{
    float scale = mat.refraction; //scale
    int check = int(floor(hit.u * scale) + floor(hit.v * scale)) % 2;
    
    vec3 color1 = mat.color;
    vec3 color2 = vec3(0.0);

    return check == 0 ? color1 : color2;
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
        vec3 mat_color = (mat.type == 3) ? getCheckerboardColor(mat, hit) : mat.color;
        
        color *= mat_color;
        light += mat.emission * mat_color;

        // if (mat.emission == 0.0)
            // light += sampleLights(hit.position, rng_state);
    }
}