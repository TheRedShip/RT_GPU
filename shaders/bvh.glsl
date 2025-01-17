

bool intersectRayBVH(Ray ray, GPUBvh node)
{
    vec3 invDir = 1.0 / ray.direction;
    
    vec3 t1 = (node.min - ray.origin) * invDir;
    vec3 t2 = (node.max - ray.origin) * invDir;
    
    vec3 tMin = min(t1, t2);
    vec3 tMax = max(t1, t2);
    
    float tEnter = max(max(tMin.x, tMin.y), tMin.z);
    float tExit = min(min(tMax.x, tMax.y), tMax.z);
    
    return tEnter <= tExit && tExit >= 0.0;
}