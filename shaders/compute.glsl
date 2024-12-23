#version 430 core

// Work group dimensions
layout(local_size_x = 16, local_size_y = 16) in;

// Output image
layout(binding = 0, rgba32f) uniform image2D outputImage;

// Uniforms for camera and scene
uniform vec2 u_resolution;
uniform vec3 u_cameraPosition;
uniform mat4 u_viewMatrix;
vec3 lightPos = vec3(5.0, 5.0, 5.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

// Scene definition
vec3 sphereCenter = vec3(0.0, 0.0, -5.0);
float sphereRadius = 1.0;
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

void main() {
    // Compute pixel coordinates
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoords.x >= int(u_resolution.x) || pixelCoords.y >= int(u_resolution.y)) {
        return;
    }

    vec2 uv = vec2(pixelCoords) / u_resolution;
    uv = uv * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    float fov = 90.0;
    float focal_length = 1.0 / tan(radians(fov) / 2.0);
    vec3 viewSpaceRay = normalize(vec3(uv.x, uv.y, -focal_length));

    vec3 rayDirection = (inverse(u_viewMatrix) * vec4(viewSpaceRay, 0.0)).xyz;
    rayDirection = normalize(rayDirection);
    Ray ray = Ray(u_cameraPosition, rayDirection);

    float t;
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    if (intersectSphere(ray, sphereCenter, sphereRadius, t)) {
        vec3 hitPoint = ray.origin + t * ray.direction;
        vec3 normal = normalize(hitPoint - sphereCenter);
        vec3 viewDir = normalize(-ray.direction);
        vec3 lighting = computeLighting(hitPoint, normal, viewDir);
        color = vec4(lighting, 1.0);
    }

    // Write to the output image
    imageStore(outputImage, pixelCoords, color);
}