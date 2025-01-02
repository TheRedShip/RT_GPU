#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;
layout(binding = 1, rgba32f) uniform image2D accumulation_image;

struct GPUObject {
	vec3    position;       // 12 + 4
	
	vec3    color;          // 12 + 4
	float   emission;       // 4
	float   roughness;      // 4
	float   specular;       // 4
	
	float   radius;         // 4
	vec3	normal;			// 12 + 4

	int     type;           // 4
};

layout(std430, binding = 1) buffer ObjectBuffer
{
	GPUObject objects[];
};

uniform int     u_objectsNum;
uniform vec2    u_resolution;
uniform vec3    u_cameraPosition;
uniform mat4    u_viewMatrix;
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

hitInfo	traceRay(Ray ray)
{
	hitInfo hit;
	hit.t = 1e30;
	hit.obj_index = -1;

	for (int i = 0; i < u_objectsNum; i++)
	{
		GPUObject obj = objects[i];

		hitInfo temp_hit;
		if (intersect(ray, obj, temp_hit) && temp_hit.t > 0.0f && temp_hit.t < hit.t)
		{
			hit.t = temp_hit.t;
			hit.obj_index = i;
			hit.position = temp_hit.position;
			hit.normal = temp_hit.normal;
		}
	}

	return (hit);
}

Ray		newRay(hitInfo hit, Ray ray, vec2 uv)
{
	Ray		new_ray;
	vec3	in_unit_sphere;

	in_unit_sphere = normalize(randomVec3(uv, u_time));
	in_unit_sphere *= sign(dot(in_unit_sphere, hit.normal));
	
	vec3 diffuse_dir = normalize(hit.normal + in_unit_sphere);
	vec3 specular_dir = reflect(ray.direction, hit.normal);
	
	new_ray.origin = hit.position + hit.normal * 0.001;
	new_ray.direction = mix(diffuse_dir, specular_dir, objects[hit.obj_index].roughness);

	return (new_ray);
}

vec3    pathtrace(Ray ray, vec2 uv)
{
	vec3	color = vec3(1.0);
	vec3	light = vec3(0.0);

	float	closest_t = 1e30;

	for (int i = 0; i < 4; i++)
	{
		hitInfo hit = traceRay(ray);
		if (hit.obj_index == -1)
		{
			light += vec3(0); //ambient color 
			break;
		}

		GPUObject obj = objects[hit.obj_index];

		color *= obj.color;
		if (obj.emission > 0.0)
		{
			light += obj.emission * obj.color;
			break;
		}

		ray = newRay(hit, ray, uv);
	}
	return (color * light);
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

	vec2 uv = (vec2(pixel_coords) / u_resolution) * 2.0 - 1.0;;
	uv.x *= u_resolution.x / u_resolution.y;

	float fov = 90.0;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));

	vec3 ray_direction = normalize((inverse(u_viewMatrix) * vec4(view_space_ray, 0.0)).xyz);
	Ray ray = Ray(u_cameraPosition, ray_direction);

	vec3 color = pathtrace(ray, uv);
	color = vec3(sqrt(color.x), sqrt(color.y), sqrt(color.z));
	
	float blend = 1.0 / float(u_frameCount + 1);
    vec4 accum = imageLoad(accumulation_image, pixel_coords);
    accum.rgb = mix(accum.rgb, color, blend);
    accum.a = 1.0;
    
    imageStore(accumulation_image, pixel_coords, accum);
    imageStore(output_image, pixel_coords, accum);
}

