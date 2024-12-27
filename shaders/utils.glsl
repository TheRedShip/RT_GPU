
float seed = 1.0;
float getRandom(vec2 uv, int frameCount)
{
    float seed = dot(uv, vec2(12.9898, 78.233)) + float(frameCount);
    return fract(sin(seed) * 43758.5453);
}
vec3 randomVec3(vec2 uv, int frameCount)
{
    return vec3(
        getRandom(uv + vec2(0.1, 0.1), frameCount),
        getRandom(uv + vec2(0.2, 0.2), frameCount),
        getRandom(uv + vec2(0.3, 0.3), frameCount)
    );
}