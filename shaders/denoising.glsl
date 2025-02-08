#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D read_texture;
layout(binding = 2, rgba32f) uniform image2D write_texture;

layout(binding = 3, rgba32f) uniform image2D position_texture;
layout(binding = 4, rgba32f) uniform image2D normal_texture;


uniform vec2    u_resolution;
uniform int     u_pass;

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

	float c_phi = 1.0;
	float p_phi = 1.0;
	float n_phi = 1.0;
	
	int holes = int(pow(2, u_pass));

	vec4 color_center = imageLoad(read_texture, pixel_coords);
    vec3 position_center = imageLoad(position_texture, pixel_coords).xyz;
    vec3 normal_center = imageLoad(normal_texture, pixel_coords).xyz;

    float kernel[5] = float[5](1.0/16.0, 1.0/4.0, 3.0/8.0, 1.0/4.0, 1.0/16.0);
    
    float totalWeight = 0.;
	vec4 color = vec4(vec3(0.), 1.0);

	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			ivec2 coords = pixel_coords + ivec2(x * holes, y * holes);
			if (coords.x < 0. || coords.y < 0. || coords.x >= int(u_resolution.x) || coords.y >= int(u_resolution.y))
				continue ;
			// coords = clamp(coords, ivec2(-1.0), u_resolution);

			vec4 color_sample = imageLoad(read_texture, coords);
			vec3 position_sample = imageLoad(position_texture, coords).xyz;
			vec3 normal_sample = imageLoad(normal_texture, coords).xyz;

			// Calculate edge-stopping weights
            
            // Color weight
            float colorDist = distance(color_center, color_sample);
            float w_c = exp(-colorDist / c_phi);
            
            // Position weight
            float posDist = distance(position_center, position_sample);
            float w_p = exp(-posDist / p_phi);

            // Normal weight
            float normalDist = distance(normal_center, normal_sample);
            float w_n = exp(-normalDist / n_phi);
            

			float weight = kernel[x+2] * kernel[y+2] * w_c * w_p * w_n;
			
			color += color_sample * weight;
			totalWeight += weight;
		}
	}

	imageStore(write_texture, pixel_coords, color / totalWeight);
}