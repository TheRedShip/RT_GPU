#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D read_texture;
layout(binding = 2, rgba32f) uniform image2D write_texture;

uniform vec2    u_resolution;

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

	vec4 color = imageLoad(read_texture, pixel_coords);

	color *= 0.5;

	imageStore(write_texture, pixel_coords, color);
}