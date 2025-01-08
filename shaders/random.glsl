#define M_PI 3.14159265359

float randomValue(inout uint rng_state)
{
	rng_state = rng_state * 747796405u + 2891336453u;
	uint result = ((rng_state >> ((rng_state >> 28u) + 4u)) ^ rng_state) * 277803737u;
	result = (result >> 22u) ^ result;
	return (float(result) * (1.0 / 4294967295.0));
}

float randomValueNormalDistribution(inout uint rng_state)
{
	float theta = 2.0 * M_PI * randomValue(rng_state);
	float rho = sqrt(-2.0 * log(randomValue(rng_state)));
	return (rho * cos(theta));
}

vec3 randomDirection(inout uint rng_state)
{
	float x = randomValueNormalDistribution(rng_state);
	float y = randomValueNormalDistribution(rng_state);
	float z = randomValueNormalDistribution(rng_state);
	return normalize(vec3(x, y, z));
}

vec2 randomPointInCircle(inout uint rng_state)
{
	float angle = randomValue(rng_state) * 2 * M_PI;
	vec2 point_in_circle = vec2(cos(angle), sin(angle));
	return (point_in_circle * sqrt(randomValue(rng_state)));
}

vec3 randomHemisphereDirection(vec3 normal, inout uint rng_state)
{
	vec3 direction = randomDirection(rng_state);
	return (direction * sign(dot(normal, direction)));
}

// vec3 GetEnvironmentLight(Ray ray)
// {
// 	vec3 sun_pos = vec3(-0.5, 0.5, 0.5);
// 	float SunFocus = 1.5;
// 	float SunIntensity = 1;

// 	vec3 GroundColour = vec3(0.5, 0.5, 0.5);
// 	vec3 SkyColourHorizon = vec3(135 / 255.0f, 206 / 255.0f, 235 / 255.0f);
// 	vec3 SkyColourZenith = SkyColourHorizon / 2.0;

// 	float skyGradientT = pow(smoothstep(0, 0.4, ray.direction.y), 0.35);
// 	float groundToSkyT = smoothstep(-0.01, 0, ray.direction.y);
// 	vec3 skyGradient = mix(SkyColourHorizon, SkyColourZenith, skyGradientT);
// 	float sun = pow(max(0, dot(ray.direction, sun_pos.xyz)), SunFocus) * SunIntensity;
// 	// Combine ground, sky, and sun
// 	vec3 composite = mix(GroundColour, skyGradient, groundToSkyT) + sun * int(groundToSkyT >= 1);
// 	return composite;
// }