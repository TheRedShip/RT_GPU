in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    // FragColor = imageLoad(screenTexture, ivec2(gl_FragCoord.xy));
    FragColor = texture(screenTexture, TexCoords);
}