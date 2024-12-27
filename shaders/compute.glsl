#version 430 core
#include "shaders/utils.glsl"

// Work group dimensions
layout(local_size_x = 16, local_size_y = 16) in;

// Output image
layout(binding = 0, rgba32f) uniform image2D outputImage;

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

vec3 lightPos = vec3(5.0, 5.0, 5.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

struct Ray {
	vec3 origin;
	vec3 direction;
};

bool intersectSphere(Ray ray, vec3 center, float radius, out float t)
{
	vec3 oc = ray.origin - center;
	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4.0 * a * c;

	if (discriminant < 0.0) {
		return false;
	}
	float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
	if (t1 > 0.001) {
		t = t1;
		return true;
	}
	return false;
}

vec3    pathtrace(Ray ray)
{
	vec3	color = vec3(0.0);
	vec3	light = vec3(0.0);

	float	closest_t = 1e30;
	for (int i = 0; i < u_objectsNum; i++)
	{
		float t;
		if (intersectSphere(ray, objects[i].position, objects[i].radius, t))
		{
			if (t < closest_t)
			{
				closest_t = t;

				vec3 hitPoint = ray.origin + t * ray.direction;
				vec3 normal = normalize(hitPoint - objects[i].position);
				
				color = objects[i].color * normal.y;
			}
		}
	}
	return (color);
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

	vec3 color = pathtrace(ray);

	imageStore(outputImage, pixelCoords, vec4(randomVec3(uv, u_frameCount), 1.0));
}

