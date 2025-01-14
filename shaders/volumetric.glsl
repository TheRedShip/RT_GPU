
struct VolumeProperties {
    vec3 sigma_a;    // absorption coefficient
    vec3 sigma_s;    // scattering coefficient
    vec3 sigma_t;    // extinction coefficient
    float g;         // phase function parameter
};

float sampleHG(float g, inout uint rng_state)
{
    if (abs(g) < 0.001)
        return 2.0 * randomValue(rng_state) - 1.0;
        
    float sqr_term = (1.0 - g * g) / (1.0 + g - 2.0 * g * randomValue(rng_state));
    return (1.0 + g * g - sqr_term * sqr_term) / (2.0 * g);
}

vec3 sampleDirection(vec3 forward, float cos_theta, inout uint rng_state)
{
    float phi = 2.0 * M_PI * randomValue(rng_state);
    float sin_theta = sqrt(max(0.0, 1.0 - cos_theta * cos_theta));
    
    vec3 dir;
    dir.x = sin_theta * cos(phi);
    dir.y = sin_theta * sin(phi);
    dir.z = cos_theta;
    
    vec3 up = abs(forward.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 right = normalize(cross(up, forward));
    up = cross(forward, right);
    
    return normalize(
        dir.x * right +
        dir.y * up +
        dir.z * forward
    );
}

bool atmosScatter(VolumeProperties volume, hitInfo hit, inout float t_scatter, inout uint rng_state)
{
    t_scatter = -log(randomValue(rng_state)) / volume.sigma_t.x;

    return (t_scatter < hit.t && volume.sigma_t.x > 0.0);
}

void calculateVolumetricLight(float t_scatter, VolumeProperties volume, inout Ray ray, inout vec3 color, inout vec3 light, inout vec3 transmittance, inout uint rng_state)
{
    vec3 scatter_pos = ray.origin + ray.direction * t_scatter;
            
    transmittance *= exp(-volume.sigma_t * t_scatter);
    color *= volume.sigma_s / volume.sigma_t;
    
    light += transmittance * color * sampleLights(scatter_pos);
    
    float cos_theta = sampleHG(volume.g, rng_state);
    vec3 new_dir = sampleDirection(ray.direction, cos_theta, rng_state);
    
    ray.origin = scatter_pos;
    ray.direction = new_dir;
}
