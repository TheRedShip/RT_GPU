#version 430 core
in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 0, rgba32f) uniform image2D screenTexture;

void main() {
    FragColor = imageLoad(screenTexture, ivec2(gl_FragCoord.xy));
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}