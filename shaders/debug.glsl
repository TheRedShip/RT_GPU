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
layout(std430, binding = 4) buffer BvhBuffer
{
	GPUBvh bvh[];
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
};

struct hitInfo
{
	float	t;
	float	last_t;
	vec3	normal;
	vec3	position;
	int		obj_index;
};

#include "shaders/intersect.glsl"
#include "shaders/bvh.glsl"

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


int traceBVHBad(Ray ray)
{
    int num_hit;

    num_hit = 0;
    for (int i = 0; i < u_bvhNum; i++)
    {
        GPUBvh node = bvh[i];

        if (intersectRayBVH(ray, node))
        {
            // num_hit++;
            for (int i = 0; i < node.primitive_count; i++)
            {
                GPUObject obj = objects[node.first_primitive + i];
                
                hitInfo tmp;
                if (intersect(ray, obj, tmp))
                    num_hit++;
            }
        }
    }

	return (num_hit);
}

int traceBVH(Ray ray)
{
    int num_hit = 0;

    const int MAX_STACK_SIZE = 64;
    int stack[MAX_STACK_SIZE];
    int stack_ptr = 0;
    
    stack[0] = 0;
    
    vec3 inv_dir = 1.0 / ray.direction;
    
    while (stack_ptr >= 0)
    {
        int current_index = stack[stack_ptr--];

        GPUBvh node = bvh[current_index];
        
        if (intersectRayBVH(ray, node))
        {
            num_hit++;

            if (node.is_leaf != 0)
            {
                for (int i = 0; i < node.primitive_count; i++)
                {
                    GPUObject obj = objects[node.first_primitive + i];
                    
                    hitInfo tmp;
                    if (intersect(ray, obj, tmp))
                        num_hit++;
                }
            }
            
            if (node.is_leaf == 0 && stack_ptr < MAX_STACK_SIZE - 2)
            {
                stack_ptr++;
                stack[stack_ptr] = node.left_index;
                stack_ptr++;
                stack[stack_ptr] = node.right_index;
            }
        }
    }
    
	return (num_hit);
}


Ray initRay(vec2 uv)
{
	float fov = camera.fov;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	
	vec3 origin = camera.position;
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));
	vec3 ray_direction = normalize((inverse(camera.view_matrix) * vec4(view_space_ray, 0.0)).xyz);

	return (Ray(origin, ray_direction));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

    vec2 uv = ((vec2(pixel_coords)) / u_resolution) * 2.0 - 1.0;;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv);
    int hits = traceBVH(ray);

    vec3 color = vec3(float(hits) / float(10));
    imageStore(output_image, pixel_coords, vec4(color, 1.));
}