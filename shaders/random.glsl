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
