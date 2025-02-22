layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;

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

	vec2	texture_vertex1;
	vec2	texture_vertex2;
	vec2	texture_vertex3;

	int		mat_index;
};
layout(std430, binding = 2) buffer TriangleBuffer
{
	GPUTriangle triangles[];
};

struct GPUBvhData
{
	mat4	transform;
	mat4	inv_transform;
	vec3	offset;
	float	scale;

	int		bvh_start_index;
	int		triangle_start_index;
};
layout(std430, binding = 3) buffer BvhDataBuffer
{
	GPUBvhData BvhData[];
};

struct GPUBvh
{
	vec3	min;
	vec3	max;

	int		index;
	int		primitive_count;
};

layout(std430, binding = 4) buffer BvhBuffer
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
	int		obj_type;

	float	u;
	float	v;
};

struct Stats
{
	int		triangle_count;
	int		box_count;
};

#include "shaders/intersect.glsl"

hitInfo traceScene(Ray ray)
{
	hitInfo hit;

	hit.t = 1e30;
	hit.obj_index = -1;
	
	for (int i = 0; i < u_objectsNum; i++)
	{
		GPUObject obj = objects[i];

		hitInfo temp_hit;
		
		if (intersect(ray, obj, temp_hit) && temp_hit.t < hit.t)
		{
			hit.t = temp_hit.t;
			hit.obj_index = i;
			hit.obj_type = obj.type;
			hit.mat_index = obj.mat_index;
			hit.position = temp_hit.position;
			hit.normal = temp_hit.normal;
		}
	}

	return (hit);
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

		if (node.primitive_count != 0)
		{
			for (int i = 0; i < node.primitive_count; i++)
			{
				GPUTriangle obj = triangles[bvh_data.triangle_start_index + node.index + i];

				stats.triangle_count += 1;
				
				hitInfo temp_hit;
				if (intersectTriangle(ray, obj, temp_hit) && temp_hit.t < hit.t)
				{
					hit.t = temp_hit.t;
					hit.obj_index = bvh_data.triangle_start_index + node.index + i;
					hit.normal = temp_hit.normal;
				}
			}
		}
		else
		{
			GPUBvh left_node = Bvh[bvh_data.bvh_start_index + current_index + 1];
			GPUBvh right_node = Bvh[bvh_data.bvh_start_index + node.index];

			hitInfo left_hit;
			hitInfo right_hit;

			left_hit.t = 1e30;
			right_hit.t = 1e30;

			bool left_bool = intersectRayBVH(ray, left_node, left_hit);
			bool right_bool = intersectRayBVH(ray, right_node, right_hit);
			
			stats.box_count += 2;

			if (left_hit.t > right_hit.t)
			{
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = current_index + 1;
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.index;
			}
			else
			{
				if (right_hit.t < hit.t && right_bool) stack[++stack_ptr] = node.index;
				if (left_hit.t < hit.t && left_bool) stack[++stack_ptr] = current_index + 1;
			}
		}
    }
    
	return (hit);
}

hitInfo traverseBVHs(Ray ray, inout Stats stats)
{
	hitInfo hit;
	
	hit.t = 1e30;
	hit.obj_index = -1;

	for (int i = 0; i < u_bvhNum; i++)
	{
		GPUBvhData bvh_data = BvhData[i];
		
		mat3 transformMatrix = mat3(bvh_data.transform);
		mat3 inverseTransformMatrix = mat3(bvh_data.inv_transform);

		Ray transformedRay;
		transformedRay.direction = normalize(transformMatrix * ray.direction);
		transformedRay.origin = transformMatrix * (ray.origin - bvh_data.offset);
		transformedRay.inv_direction = (1. / transformedRay.direction);
		
		hitInfo temp_hit = traceBVH(transformedRay, BvhData[i], stats);

		float transformed_t = temp_hit.t * (1.0 / bvh_data.scale);
		if (transformed_t < hit.t)
		{
			GPUTriangle triangle = triangles[temp_hit.obj_index];

			hit.u = temp_hit.u;
			hit.v = temp_hit.v;
			hit.t = transformed_t;
			hit.obj_index = temp_hit.obj_index;
			hit.mat_index = triangle.mat_index;
		
			vec3 position = transformedRay.origin + transformedRay.direction * temp_hit.t;
			hit.position = inverseTransformMatrix * position + bvh_data.offset;
			
			vec3 based_normal = triangle.normal * sign(-dot(transformedRay.direction, triangle.normal));
			hit.normal = normalize(inverseTransformMatrix * based_normal);
		}
	}

	hit.obj_type = 3;

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

hitInfo trace(Ray ray, inout Stats stats)
{
	hitInfo hitBVH;
	hitInfo hitScene;
	hitInfo hit;

	hitBVH = traverseBVHs(ray, stats);
	hitScene = traceScene(ray);
	return (hitBVH.t < hitScene.t ? hitBVH : hitScene);
}

vec3 debugColor(vec2 uv)
{
	Ray ray = initRay(uv);
	Stats stats = Stats(0, 0);

	hitInfo hit = trace(ray, stats);

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

    vec2 uv = ((vec2(pixel_coords)) * (1.0 / u_resolution)) * 2.0 - 1.0;;
	uv.x *= u_resolution.x * (1.0 / u_resolution.y);

	vec3 color = debugColor(uv);

	imageStore(output_image, pixel_coords, vec4(color, 1.));
}