float randomValue(inout uint rng_state)
{
	rng_state = rng_state * 747796405 + 2891336453;
	uint result = ((rng_state >> ((rng_state >> 28) + 4)) ^ rng_state) * 277803737;
	result = (result >> 22) ^ result;
	return (result / 4294967295.0);
}

float randomValueNormalDistribution(inout uint rng_state)
{
	float theta = 2.0 * 3.14159265359 * randomValue(rng_state);
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

vec3 randomHemisphereDirection(vec3 normal, inout uint rng_state)
{
	vec3 direction = randomDirection(rng_state);
	return (direction * sign(dot(normal, direction)));
}