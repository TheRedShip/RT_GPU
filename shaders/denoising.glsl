layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_texture;
layout(binding = 2, rgba32f) uniform image2D write_texture;

layout(binding = 3, rgba32f) uniform image2D normal_texture;
layout(binding = 4, rgba32f) uniform image2D position_texture;
layout(binding = 5, rgba32f) uniform image2D light_texture;
layout(binding = 7, rgba32f) uniform image2D color_texture;


uniform vec2    u_resolution;
uniform int		u_pass_count;
uniform int     u_pass;

uniform float   u_c_phi;
uniform float   u_p_phi;
uniform float   u_n_phi;


void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y)) 
		return;

	int holes = int(pow(2, u_pass));

	vec4 light_center = imageLoad(light_texture, pixel_coords);
    vec3 position_center = imageLoad(position_texture, pixel_coords).xyz;
    vec3 normal_center = imageLoad(normal_texture, pixel_coords).xyz;

    float kernel[5] = float[5](1.0/16.0, 1.0/4.0, 3.0/8.0, 1.0/4.0, 1.0/16.0);
    
    float totalWeight = 0.;
	vec4 light = vec4(vec3(0.), 1.0);

	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			ivec2 coords = pixel_coords + ivec2(x * holes, y * holes);
			if (coords.x < 0. || coords.y < 0. || coords.x >= int(u_resolution.x) || coords.y >= int(u_resolution.y))
				continue ;
			// coords = clamp(coords, ivec2(-1.0), u_resolution);

			vec4 light_sample = imageLoad(light_texture, coords);
			vec3 position_sample = imageLoad(position_texture, coords).xyz;
			vec3 normal_sample = imageLoad(normal_texture, coords).xyz;

			// Calculate edge-stopping weights
            
            // light weight
            float lightDist = distance(light_center, light_sample);
            float w_c = exp(-lightDist / u_c_phi);
            
            // Position weight
            float posDist = distance(position_center, position_sample);
            float w_p = exp(-posDist / u_p_phi);

            // Normal weight
            float normalDist = distance(normal_center, normal_sample);
            float w_n = exp(-normalDist / u_n_phi);

			float weight = kernel[x+2] * kernel[y+2] * w_c * w_p * w_n;
			
			light += light_sample * weight;
			totalWeight += weight;
		}
	}
	light * (1.0 / totalWeight);
	if (u_pass == u_pass_count - 1)
	{
		vec4 color = light * imageLoad(color_texture, pixel_coords);
		imageStore(output_texture, pixel_coords, sqrt(color));
		return;
	}

	imageStore(write_texture, pixel_coords, light);
}