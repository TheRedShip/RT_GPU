#version 430 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    vec2 uv = TexCoords;

    float incr_x = 1.0 / 1920.0 ;
    float incr_y = 1.0 / 1080.0 ;

    int iteration = 5;
    
    if (iteration == 0) {FragColor = texture(screenTexture, uv);return;}

    vec4 color = vec4(vec3(0.), 1.0);
    float totalWeight = 0.;

    float kernel[5] = float[5](1.0/16.0, 1.0/4.0, 3.0/8.0, 1.0/4.0, 1.0/16.0);
    
    for (int i = 0; i < iteration; i++)
    {
        int holes = int(pow(2, i));

        for (int x = -2; x <= 2; x++)
        {
            for (int y = -2; y <= 2; y++)
            {
                vec2 current_uv = uv + vec2(x * holes * incr_x, y * holes * incr_y);
                // if (current_uv.x < 0. || current_uv.y < 0. || current_uv.x > 1. || current_uv.y > 1.)
                    // continue ;
                // current_uv = clamp(current_uv, vec2(0.), vec2(1.));

                float weight = kernel[x+2] * kernel[y+2];
                
                totalWeight += weight;
                color += texture(screenTexture, current_uv) * weight;
            }
        }
    }


    FragColor = color / totalWeight;
}