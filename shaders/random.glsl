float randomValue(inout uint rng_state)
{
	rng_state = rng_state * 747796405u + 2891336453u;
	uint result = ((rng_state >> ((rng_state >> 28u) + 4u)) ^ rng_state) * 277803737u;
	result = (result >> 22u) ^ result;
	return (float(result) * (1.0 / 4294967295.0));
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

#define M_PI 3.1415926535897932384626433832795

// vec3 randomHemisphereDirection(vec3 normal, inout uint rng_state)
// {
//     float r1 = randomValue(rng_state);
//     float r2 = randomValue(rng_state);
    
//     float phi = 2.0 * M_PI * r1;
//     float cos_theta = sqrt(1.0 - r2);
//     float sin_theta = sqrt(r2);
    
//     // Create orthonormal basis
//     vec3 tangent, bitangent;
//     if (abs(normal.x) > abs(normal.z)) {
//         tangent = normalize(vec3(-normal.y, normal.x, 0.0));
//     } else {
//         tangent = normalize(vec3(0.0, -normal.z, normal.y));
//     }
//     bitangent = cross(normal, tangent);
    
//     return normalize(
//         tangent * (cos(phi) * sin_theta) +
//         bitangent * (sin(phi) * sin_theta) +
//         normal * cos_theta
//     );
// }