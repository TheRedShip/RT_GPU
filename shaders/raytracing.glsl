
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_texture;
layout(binding = 1, rgba32f) uniform image2D output_accum_texture;

layout(binding = 3, rgba32f) uniform image2D normal_texture;
layout(binding = 4, rgba32f) uniform image2D position_texture;
layout(binding = 5, rgba32f) uniform image2D light_texture;
layout(binding = 6, rgba32f) uniform image2D light_accum_texture;
layout(binding = 7, rgba32f) uniform image2D color_texture;

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

	vec3	normal_vertex1;
	vec3	normal_vertex2;
	vec3	normal_vertex3;

	vec2	texture_vertex1;
	vec2	texture_vertex2;
	vec2	texture_vertex3;

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
	int		texture_index;	// 4
	int		emission_texture_index;	// 4
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
	mat4	inv_transform;
	vec3	offset;
	float	scale;

	int		bvh_start_index;
	int		triangle_start_index;
};

struct GPUBvh
{
	vec3	min;
	vec3	max;

	int		index;
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
	int		obj_type;

	float	u;
	float	v;
};

#include "shaders/random.glsl"
#include "shaders/intersect.glsl"
#include "shaders/scatter.glsl"
#include "shaders/light.glsl"
#include "shaders/volumetric.glsl"
#include "shaders/trace.glsl"

#define accumulate(texture, new_color, color) imageLoad(texture, ivec2(gl_GlobalInvocationID.xy)); new_color.rgb = mix(new_color.rgb, color, 1.0 / float(u_frameCount + 1)); new_color.a = 1.0; imageStore(texture, ivec2(gl_GlobalInvocationID.xy), new_color);

vec3[2] pathtrace(Ray ray, inout uint rng_state)
{
    vec3 color = vec3(1.0);
    vec3 light = vec3(0.0);
    vec3 transmittance = vec3(1.0);
    
    for (int i = 0; i < camera.bounce; i++)
    {
        hitInfo hit = traceRay(ray);

		#if SHADER_FOG
			float t_scatter = 0.0;
			bool scatter_valid = bool(volume.enabled != 0 && atmosScatter(hit, t_scatter, rng_state));
			if (scatter_valid)
			{
				calculateVolumetricLight(t_scatter, ray, color, light, transmittance, rng_state);
				continue ;
			}
		#endif

		if (hit.obj_index == -1)
		{
			light += transmittance * GetEnvironmentLight(ray);
			break;
		}

		if (i == 0)
		{
			// imageStore(normal_texture, ivec2(gl_GlobalInvocationID.xy), vec4(normalize(hit.normal), 1.0));
			// imageStore(position_texture, ivec2(gl_GlobalInvocationID.xy), vec4(normalize(hit.position), 1.0));
			vec4 accum_normal = accumulate(normal_texture, accum_normal, normalize(hit.normal));
			vec4 accum_position = accumulate(position_texture, accum_position, normalize(hit.position));
		}

        float p = max(color.r, max(color.g, color.b));
        if (randomValue(rng_state) >= p) break;
		color /= max(p, 0.001);

        GPUMaterial mat = materials[hit.mat_index];
        calculateLightColor(mat, hit, color, light, rng_state);
        
		if (mat.emission > 0.0 && mat.emission_texture_index == -1)
			break;

        ray = newRay(hit, ray, rng_state);
        ray.inv_direction = 1.0 / ray.direction;
    }
    
	vec3[2] result = {color, light};
    return result;
}

Ray initRay(vec2 uv, inout uint rng_state)
{
	float focal_length = 1.0 / tan(radians(camera.fov) / 2.0);
	
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

	vec2 uv = ((vec2(pixel_coords) + jitter) / u_resolution) * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv, rng_state);
	vec3[2] color_light = pathtrace(ray, rng_state);
	vec3 color = color_light[0] * color_light[1];
	
	vec4 accum_color = accumulate(color_texture, accum_color, (color_light[0]));
	vec4 accum_light = accumulate(light_accum_texture, accum_light, color_light[1]);

	imageStore(light_texture, pixel_coords, accum_light);

	vec4 accum = accumulate(output_accum_texture, accum, color);
	vec4 final_output_color = sqrt(accum);
	
	for (int y = 0; y < u_pixelisation; y++)
		for (int x = 0; x < u_pixelisation; x++)
			imageStore(output_texture, pixel_coords + ivec2(x, y), final_output_color);
}

