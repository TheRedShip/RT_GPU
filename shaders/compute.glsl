#version 430 core

#include "shaders/random.glsl"

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 1, rgba32f) uniform image2D accumulationImage;

struct GPUObject {
	vec3    position;       // 12 + 4
	vec3    color;          // 12 + 4
	float   emission;       // 4
	float   roughness;      // 4
	float   specular;       // 4
	float   radius;         // 4
	int     type;           // 4 + 12
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

vec3 lightPos = vec3(5.0, 5.0, 5.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

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

bool intersectSphere(Ray ray, GPUObject obj, out hitInfo hit)
{
	vec3 oc = ray.origin - obj.position;
	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - obj.radius * obj.radius;
	float discriminant = b * b - 4.0 * a * c;

	if (discriminant < 0.0)
		return false;

	float t = (-b - sqrt(discriminant)) / (2.0 * a);
	if (t < 0.0)
		t = (-b + sqrt(discriminant)) / (2.0 * a);

	hit.t = t;
	hit.position = ray.origin + ray.direction * t;
	hit.normal = normalize(hit.position - obj.position);

	return (true);
}

hitInfo	trace_ray(Ray ray)
{
	hitInfo hit;
	hit.t = 1e30;
	hit.obj_index = -1;

	for (int i = 0; i < u_objectsNum; i++)
	{
		GPUObject obj = objects[i];

		hitInfo tempHit;
		if (intersectSphere(ray, obj, tempHit))
		{
			if (tempHit.t > 0.0f && tempHit.t < hit.t)
			{
				hit.t = tempHit.t;
				hit.obj_index = i;
				hit.position = tempHit.position;
				hit.normal = tempHit.normal;
			}
		}
	}

	return (hit);
}

vec3    pathtrace(Ray ray, vec2 random)
{
	vec3	color = vec3(1.0);
	vec3	light = vec3(0.0);

	float	closest_t = 1e30;
	for (int i = 0; i < 10; i++)
	{
		hitInfo hit = trace_ray(ray);
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

		ray.origin = hit.position + hit.normal * 0.001;
		//cosine weighted importance sampling
		vec3 unit_sphere = normalize(randomVec3(random, u_time));
		if (dot(unit_sphere, hit.normal) < 0.0)
			unit_sphere = -unit_sphere;
		ray.direction = normalize(hit.normal + unit_sphere);
	}
	return (color * light);
}

void main() {
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	if (pixelCoords.x >= int(u_resolution.x) || pixelCoords.y >= int(u_resolution.y)) 
		return;

	vec2 uv = vec2(pixelCoords) / u_resolution;
	uv = uv * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	float fov = 90.0;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	vec3 viewSpaceRay = normalize(vec3(uv.x, uv.y, -focal_length));

	vec3 rayDirection = (inverse(u_viewMatrix) * vec4(viewSpaceRay, 0.0)).xyz;
	rayDirection = normalize(rayDirection);
	Ray ray = Ray(u_cameraPosition, rayDirection);

	vec3 color = pathtrace(ray, uv);
	
	vec4 accum = imageLoad(accumulationImage, pixelCoords);
    accum.rgb = accum.rgb * float(u_frameCount) / float(u_frameCount + 1) + color / float(u_frameCount + 1);
    accum.a = 1.0;
    
    imageStore(accumulationImage, pixelCoords, accum);
    imageStore(outputImage, pixelCoords, accum);
}

