#version 430 core
out vec4        FragColor;

uniform vec2    u_resolution;

void main()
{
    vec2    uv;
    vec4    color;

    uv = gl_FragCoord.xy / u_resolution.xy;
    uv.x *= u_resolution.x / u_resolution.y;

    color = vec4(uv.x, uv.y, 0., 0.);

    FragColor = color;
}