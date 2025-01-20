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

struct GPUTriangle
{
	vec3	position;
	vec3	vertex1;
	vec3	vertex2;
	vec3	normal;

	int		mat_index;
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

struct GPUBvhData
{
	mat4	transform;
	vec3	offset;
	float	scale;

	int		bvh_start_index;
	int		triangle_start_index;
};

struct GPUBvh
{
	vec3	min;
	vec3	max;

	int		left_index;	
	int		right_index;

	int		is_leaf;
	
	int		first_primitive;
	int		primitive_count;
};

layout(std430, binding = 1) buffer ObjectBuffer
{
	GPUObject objects[];
};

layout(std430, binding = 2) buffer TriangleBuffer
{
	GPUTriangle triangles[];
};

layout(std430, binding = 3) buffer BvhDataBuffer
{
	GPUBvhData BvhData[];
};

layout(std430, binding = 4) buffer BvhBuffer
{
	GPUBvh Bvh[];
};

layout(std430, binding = 5) buffer MaterialBuffer
{
	GPUMaterial materials[];
};

layout(std430, binding = 6) buffer LightsBuffer
{
    int lightsIndex[];
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
uniform int     u_bvhNum;
uniform int     u_lightsNum;
uniform vec2    u_resolution;
uniform int		u_pixelisation;
uniform int		u_frameCount;
uniform float	u_time;

struct Ray
{
	vec3 origin;
	vec3 direction;
	vec3 inv_direction;
};

struct hitInfo
{
	float	t;
	float	last_t;
	vec3	normal;
	vec3	position;
	int		obj_index;
	int		mat_index;
};

#include "shaders/random.glsl"
#include "shaders/intersect.glsl"
#include "shaders/scatter.glsl"
#include "shaders/light.glsl"
#include "shaders/volumetric.glsl"
#include "shaders/trace.glsl"


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

		GPUMaterial mat = materials[hit.mat_index];
		
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
		ray.inv_direction = 1.0 / ray.direction;
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

	return (Ray(origin, ray_direction, 1.0 / ray_direction));
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

