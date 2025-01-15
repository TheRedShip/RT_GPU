#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;
layout(binding = 1, rgba32f) uniform image2D accumulation_image;

struct GPUObject {
	mat4	rotation;

	vec3    position;       // 12 + 4
	
	vec3	normal;			// 12 + 4
	
	vec3	vertex1;		// 12 + 4
	vec3	vertex2;		// 12 + 4

	float   radius;         // 4

	int		mat_index;		// 4
	int     type;           // 4
};

struct GPUMaterial
{
	vec3    color;          // 12 + 4
	float   emission;       // 4
	float   roughness;      // 4
	float   metallic;       // 4
	float	refraction;		// 4
	int		type;			// 4
};

struct GPUCamera
{
	mat4	view_matrix;
    vec3	position;
	
	float	aperture_size;
	float	focus_distance;
	float	fov;

	int		bounce;
};

struct GPUVolume
{
	vec3	sigma_a;    // absorption coefficient
	vec3	sigma_s;    // scattering coefficient
	vec3	sigma_t;    // extinction coefficient
	float	g;         // phase function parameter
	int		enabled;
};

layout(std430, binding = 1) buffer ObjectBuffer
{
	GPUObject objects[];
};

layout(std430, binding = 2) buffer MaterialBuffer
{
	GPUMaterial materials[];
};

layout(std140, binding = 0) uniform CameraData
{
    GPUCamera camera;
};

layout(std140, binding = 1) uniform VolumeData
{
    GPUVolume volume;
};

uniform int     u_objectsNum;
uniform vec2    u_resolution;
uniform int		u_pixelisation;
uniform int		u_frameCount;
uniform float	u_time;

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct hitInfo
{
	float	t;
	vec3	normal;
	vec3	position;
	int		obj_index;
};

#include "shaders/random.glsl"
#include "shaders/intersect.glsl"
#include "shaders/scatter.glsl"
#include "shaders/light.glsl"
#include "shaders/volumetric.glsl"

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

hitInfo	traceRay(Ray ray)
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

vec3    pathtrace(Ray ray, inout uint rng_state)
{
	vec3	color = vec3(1.0);
	vec3	light = vec3(0.0);
	
	vec3 transmittance = vec3(1.0);

	for (int i = 0; i < camera.bounce; i++)
	{
		hitInfo hit = traceRay(ray);

		float t_scatter = 0.0;
		if (volume.enabled != 0 && atmosScatter(hit, t_scatter, rng_state))
        {
			calculateVolumetricLight(t_scatter, ray, color, light, transmittance, rng_state);
            continue;
        }

		if (hit.obj_index == -1)
		{
			light += transmittance * GetEnvironmentLight(ray);
			break;
		}

		if (volume.enabled != 0)
			transmittance *= exp(-volume.sigma_t * hit.t);

		GPUObject obj = objects[hit.obj_index];
		GPUMaterial mat = materials[obj.mat_index];
		
		// RR
		float p = max(color.r, max(color.g, color.b));
        if (randomValue(rng_state) > p)
            break;
        color /= p;
		//

		calculateLightColor(mat, hit, color, light, rng_state);
		
		if (mat.emission > 0.0)
			break;

		ray = newRay(hit, ray, rng_state);
	}

	return (color * light);
}

Ray initRay(vec2 uv, inout uint rng_state)
{
	float fov = camera.fov;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	
	vec3 origin = camera.position;
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));
	vec3 ray_direction = normalize((inverse(camera.view_matrix) * vec4(view_space_ray, 0.0)).xyz);
	
	vec3 right = vec3(camera.view_matrix[0][0], camera.view_matrix[1][0], camera.view_matrix[2][0]);
	vec3 up = vec3(camera.view_matrix[0][1], camera.view_matrix[1][1], camera.view_matrix[2][1]);

	vec3 focal_point = origin + ray_direction * camera.focus_distance;

	float r = sqrt(randomValue(rng_state));
	float theta = 2.0 * M_PI * randomValue(rng_state);
	vec2 lens_point = camera.aperture_size * r * vec2(cos(theta), sin(theta));

	origin += right * lens_point.x + up * lens_point.y;
	ray_direction = normalize(focal_point - origin);

	return (Ray(origin, ray_direction));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

	if (u_pixelisation != 1 && (uint(pixel_coords.x) % u_pixelisation != 0 || uint(pixel_coords.y) % u_pixelisation != 0))
		return;

	uint rng_state = uint(u_resolution.x) * uint(pixel_coords.y) + uint(pixel_coords.x);
	rng_state = rng_state + u_frameCount * 719393;

	vec2 jitter = randomPointInCircle(rng_state) * 1;

	vec2 uv = ((vec2(pixel_coords) + jitter) / u_resolution) * 2.0 - 1.0;;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv, rng_state);
	vec3 color = pathtrace(ray, rng_state);
	
	float blend = 1.0 / float(u_frameCount + 1);
    vec4 accum = imageLoad(accumulation_image, pixel_coords);
    accum.rgb = mix(accum.rgb, color, blend);
	accum.a = 1.0;

    imageStore(accumulation_image, pixel_coords, accum);
    
    vec4 final_color = vec4(sqrt(accum.r), sqrt(accum.g), sqrt(accum.b), accum.a);

	for (int y = 0; y < u_pixelisation; y++)
		for (int x = 0; x < u_pixelisation; x++)
			imageStore(output_image, pixel_coords + ivec2(x, y), final_color);
}

