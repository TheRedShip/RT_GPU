
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

vec3 randomVec3Mixed(vec2 uv, int frameCount, float min_val, float max_val)
{
    float randomX = getRandom(uv + vec2(0.1, 0.1), frameCount);
    float randomY = getRandom(uv + vec2(0.2, 0.2), frameCount);
    float randomZ = getRandom(uv + vec2(0.3, 0.3), frameCount);
    
    return vec3(
        mix(min_val, max_val, randomX),
        mix(min_val, max_val, randomY),
        mix(min_val, max_val, randomZ)
    );
}