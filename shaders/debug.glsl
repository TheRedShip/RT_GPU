#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;
layout(binding = 1, rgba32f) uniform image2D accumulation_image;

struct GPUCamera
{
	mat4	view_matrix;
    vec3	position;
	
	float	aperture_size;
	float	focus_distance;
	float	fov;

	int		bounce;
};
layout(std140, binding = 0) uniform CameraData
{
    GPUCamera camera;
};

struct GPUDebug
{
	int	enabled;
	int	mode;
	int	triangle_treshold;
	int	box_treshold;
};
layout(std140, binding = 2) uniform DebugData
{
    GPUDebug debug;
};

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
layout(std430, binding = 1) buffer ObjectBuffer
{
	GPUObject objects[];
};


struct GPUTriangle
{
	vec3	position;
	vec3	vertex1;
	vec3	vertex2;
	vec3	normal;

	int		mat_index;
};
layout(std430, binding = 2) buffer TriangleBuffer
{
	GPUTriangle triangles[];
};

struct GPUTopBvh
{
	vec3	min;
	vec3	max;

	int		left_index;
	int		right_index;

	int		is_leaf;
	
	int		first_bvh;
	int		bvh_count;
};
layout(std430, binding = 3) buffer TopBvhBuffer
{
	GPUTopBvh TopBvh[];
};

struct GPUBvhData
{
	mat4	transform;
	vec3	offset;
	float	scale;

	int		bvh_start_index;
	int		triangle_start_index;
};
layout(std430, binding = 4) buffer BvhDataBuffer
{
	GPUBvhData BvhData[];
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
layout(std430, binding = 5) buffer BvhBuffer
{
	GPUBvh Bvh[];
};

uniform int     u_objectsNum;
uniform int     u_bvhNum;
uniform vec2    u_resolution;
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

struct Stats
{
	int		triangle_count;
	int		box_count;
};

#include "shaders/intersect.glsl"

int traceRay(Ray ray)
{
    int num_hit;

    num_hit = 0;
    for (int i = 0; i < u_objectsNum; i++)
    {
        GPUObject obj = objects[i];

        hitInfo temp_hit;
        if (intersect(ray, obj, temp_hit))
            num_hit++;
    }

	return (num_hit);
}

hitInfo traceBVH(Ray ray, GPUBvhData bvh_data, inout Stats stats)
{
	hitInfo hit;
	hitInfo hit_bvh;

	hit.t = 1e30;
	hit.obj_index = -1;

    int stack[32];
    int stack_ptr = 0;
    stack[0] = 0;
    
    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];
        GPUBvh node = Bvh[bvh_data.bvh_start_index + current_index];

		if (node.is_leaf != 0)
		{
			for (int i = 0; i < node.primitive_count; i++)
			{
				GPUTriangle obj = triangles[bvh_data.triangle_start_index + node.first_primitive + i];
				
				hitInfo temp_hit;
				if (intersectTriangle(ray, obj, temp_hit) && temp_hit.t < hit.t)
				{
					hit.t = temp_hit.t;
					hit.last_t = temp_hit.last_t;
					hit.obj_index = bvh_data.triangle_start_index + node.first_primitive + i;
					hit.mat_index = obj.mat_index;
					hit.position = temp_hit.position;
					hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUBvh left_node = Bvh[bvh_data.bvh_start_index + node.left_index];
			GPUBvh right_node = Bvh[bvh_data.bvh_start_index + node.right_index];

			hitInfo left_hit;
			hitInfo right_hit;

			left_hit.t = 1e30;
			right_hit.t = 1e30;

			bool left_bool = intersectRayBVH(ray, left_node.min, left_node.max, left_hit);
			bool right_bool = intersectRayBVH(ray, right_node.min, left_node.max, right_hit);

			if (left_hit.t > right_hit.t)
			{
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
			}
			else
			{
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
			}
		}
    }
    
	return (hit);
}


hitInfo traverseBVHs(Ray ray, GPUBvhData bvh_data, inout Stats stats)
{
	hitInfo	hit;

	hit.t = 1e30;
	hit.obj_index = -1;

	mat3 transformMatrix = mat3(bvh_data.transform);
	mat3 inverseTransformMatrix = inverse(transformMatrix);

	Ray transformedRay;
	transformedRay.direction = normalize(transformMatrix * ray.direction);
	transformedRay.origin = transformMatrix * (ray.origin - bvh_data.offset);
	transformedRay.inv_direction = (1. / transformedRay.direction);
	
	hit = traceBVH(transformedRay, bvh_data, stats);

	if (hit.obj_index == -1)
		return (hit);

	hit.t = hit.t / bvh_data.scale;
	hit.last_t = hit.last_t / bvh_data.scale;
	hit.obj_index = hit.obj_index;
	hit.mat_index = hit.mat_index;
	hit.position = inverseTransformMatrix * hit.position + bvh_data.offset;
	hit.normal = normalize(inverseTransformMatrix * hit.normal);

	return (hit);
}

hitInfo traceTopBVH(Ray ray, inout Stats stats)
{
	hitInfo hit;
	hitInfo hit_bvh;

	hit.t = 1e30;
	hit.obj_index = -1;

    int stack[32];
    int stack_ptr = 0;
    stack[0] = 0;
    
    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];
        GPUTopBvh node = TopBvh[current_index];


		if (node.is_leaf != 0)
		{
			for (int i = 0; i < node.bvh_count; i++)
			{
				GPUBvhData bvh_data = BvhData[node.first_bvh + i];
				
				hitInfo temp_hit = traverseBVHs(ray, bvh_data, stats);
				if (temp_hit.obj_index != -1 && temp_hit.t < hit.t)
				{
					hit.t = temp_hit.t;
					hit.last_t = temp_hit.last_t;
					hit.obj_index = temp_hit.obj_index;
					hit.mat_index = temp_hit.mat_index;
					hit.position = temp_hit.position;
					hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUTopBvh left_node = TopBvh[node.left_index];
			GPUTopBvh right_node = TopBvh[node.right_index];

			hitInfo left_hit;
			hitInfo right_hit;

			left_hit.t = 1e30;
			right_hit.t = 1e30;

			stats.box_count += 2;

			bool left_bool = intersectRayBVH(ray, left_node.min, left_node.max, left_hit);
			bool right_bool = intersectRayBVH(ray, right_node.min, right_node.max, right_hit);

			if (left_hit.t > right_hit.t)
			{
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
			}
			else
			{
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.right_index;
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = node.left_index;
			}
		}
    }
    
	return (hit);
}

Ray initRay(vec2 uv)
{
	float fov = camera.fov;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	
	vec3 origin = camera.position;
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));
	vec3 ray_direction = normalize((inverse(camera.view_matrix) * vec4(view_space_ray, 0.0)).xyz);

	return (Ray(origin, ray_direction, (1.0 / ray_direction)));
}

vec3 debugColor(vec2 uv)
{
	Ray ray = initRay(uv);
	Stats stats = Stats(0, 0);

	hitInfo hit = traceTopBVH(ray, stats);
	
	float box_display = float(stats.box_count) / float(debug.box_treshold);
	float triangle_display = float(stats.triangle_count) / float(debug.triangle_treshold);

	switch (debug.mode)
	{
		case 0:
			return (hit.normal * 0.5 + 0.5) * int(hit.obj_index != -1);
		case 1:
			return (box_display < 1. ? vec3(box_display) : vec3(1., 0., 0.));
		case 2:
			return (triangle_display < 1. ? vec3(triangle_display) : vec3(1., 0., 0.));
	}

	return (vec3(0.));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

    vec2 uv = ((vec2(pixel_coords)) / u_resolution) * 2.0 - 1.0;;
	uv.x *= u_resolution.x / u_resolution.y;
	
	vec3 color = debugColor(uv);

	imageStore(output_image, pixel_coords, vec4(color, 1.));
}