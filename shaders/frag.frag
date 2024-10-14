#version 430 core
out vec4        FragColor;

uniform vec2    u_resolution;

vec3 sphereCenter = vec3(0.0, 0.0, -5.0);
float sphereRadius = 1.0;


vec3 lightPos = vec3(5.0, 5.0, 5.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 objectColor = vec3(0.4, 0.7, 0.9); 


struct Ray {
    vec3 origin;
    vec3 direction;
};


bool intersectSphere(Ray ray, vec3 center, float radius, out float t) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0.0) {
        return false;
    } else {
        t = (-b - sqrt(discriminant)) / (2.0 * a);
        return true;
    }
}


vec3 computeLighting(vec3 point, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(lightPos - point);
    float diff = max(dot(normal, lightDir), 0.0);
    
    
    vec3 diffuse = diff * lightColor;

    
    return objectColor * diffuse;
}


void    main()
{
	vec2    uv;
	vec4    color;

	uv = gl_FragCoord.xy / u_resolution.xy * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	vec3 rayOrigin = vec3(0.0, 0.0, 0.0); 
    vec3 rayDirection = normalize(vec3(uv, -1.0)); 

    Ray ray = Ray(rayOrigin, rayDirection);
    
    float t;
    if (intersectSphere(ray, sphereCenter, sphereRadius, t))
	{
        vec3 hitPoint = ray.origin + t * ray.direction;
        vec3 normal = normalize(hitPoint - sphereCenter);

        vec3 viewDir = normalize(-ray.direction);

        vec3 color = computeLighting(hitPoint, normal, viewDir);
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
    }
}