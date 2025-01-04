float rand_seed = 0;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; 
    const uint ieeeOne      = 0x3F800000u; 

    m &= ieeeMantissa;
    m |= ieeeOne;

    float  f = uintBitsToFloat( m );       
    return f - 1.0;                        
}


float randombis( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float randombis( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float randombis( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float randombis( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

float random(vec2 uv, float time)
{
	if (rand_seed == 0)
		rand_seed = time;
	rand_seed *= 2;
	return randombis(vec3(uv.xy, time * rand_seed));
}

vec3 randomVec3(vec2 uv, float time)
{
    return (vec3((random(uv, time) - 0.5) * 2, (random(uv, time) - 0.5) * 2, (random(uv, time) - 0.5) * 2));
}

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