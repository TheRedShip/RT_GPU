bool intersectSphere(Ray ray, GPUObject obj, out hitInfo hit)
{
    vec3 oc = ray.origin - obj.position;
    
    float b = dot(oc, ray.direction);
    float c = dot(oc, oc) - obj.radius * obj.radius;
    float h = b * b - c;
    
    float t = -b - sqrt(h);
    t = mix(t, -b + sqrt(h), step(t, 0.0));
    
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    hit.normal = normalize(hit.position - obj.position);
    
    return (h >= 0.0 && t > 0.0);
}

bool intersectPlane(Ray ray, GPUObject obj, out hitInfo hit)
{
    float d = dot(obj.normal, ray.direction);
    float t = dot(obj.position - ray.origin, obj.normal) / d;
    bool valid = t >= 0.0 && d != 0.0;

    if (!valid) return (false);
    
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    hit.normal = d < 0.0 ? obj.normal : -obj.normal;
    
    return (valid);
}

bool intersectQuad(Ray ray, GPUObject obj, out hitInfo hit)
{
    vec3 normal = normalize(cross(obj.vertex1, obj.vertex2));
    float d = dot(normal, ray.direction);

    if (d == 0.0) return (false);

    float t = dot(obj.position - ray.origin, normal) / d;

    if (t <= 0.0) return (false);
    
    vec3 p = ray.origin + ray.direction * t - obj.position;
    
    float e1 = dot(p, obj.vertex1);
    float e2 = dot(p, obj.vertex2);
    
    float l1 = dot(obj.vertex1, obj.vertex1);
    float l2 = dot(obj.vertex2, obj.vertex2);
    
    bool inside = e1 >= 0.0 && e1 <= l1 && e2 >= 0.0 && e2 <= l2;
    
    hit.t = t;
    hit.position = p + obj.position;
    hit.normal = normal * -sign(d);
    
    return (inside);
}

// bool intersectTriangle(Ray ray, GPUObject obj, out hitInfo hit)
// {
//     vec3 pvec = cross(ray.direction, obj.vertex2);
//     float det = dot(obj.vertex1, pvec);
//     vec3 tvec = ray.origin - obj.position;
    
//     float invDet = 1.0 / det;
//     float u = dot(tvec, pvec) * invDet;
//     vec3 qvec = cross(tvec, obj.vertex1);
//     float v = dot(ray.direction, qvec) * invDet;
//     float t = dot(obj.vertex2, qvec) * invDet;
    
//     bool valid = abs(det) > 1e-8 &&
//                 u >= 0.0 && u <= 1.0 &&
//                 v >= 0.0 && (u + v) <= 1.0 &&
//                 t > 0.0;
    
//     hit.t = t;
//     hit.position = ray.origin + ray.direction * t;
//     hit.normal = obj.normal * sign(-dot(ray.direction, obj.normal));
    
//     return (valid);
// }

bool intersectTriangle(Ray ray, GPUObject obj, out hitInfo hit)
{
    vec3 pvec = cross(ray.direction, obj.vertex2);
    float det = dot(obj.vertex1, pvec);
    
    if (abs(det) < 1e-8) return (false); // det < 0.0
    
    float invDet = 1.0 / det;
    
    vec3 tvec = ray.origin - obj.position;
    
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) return (false);
    
    vec3 qvec = cross(tvec, obj.vertex1);
    float v = dot(ray.direction, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) return (false);
    
    float t = dot(obj.vertex2, qvec) * invDet;
    if (t <= 0.0) return (false);
    
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    
    vec3 normal = obj.normal;
    hit.normal = dot(ray.direction, normal) < 0.0 ? normal : -normal;
        
    return (true);
}

bool intersectCube(Ray ray, GPUObject obj, out hitInfo hit)
{
    vec3 obj_size = obj.vertex1;
    vec3 halfSize = obj_size * 0.5;
    vec3 rayOriginLocal = ray.origin - obj.position;
    
    vec3 invDir = 1.0 / ray.direction;
    vec3 t1 = (-halfSize - rayOriginLocal) * invDir;
    vec3 t2 = (halfSize - rayOriginLocal) * invDir;
    
    vec3 tMinVec = min(t1, t2);
    vec3 tMaxVec = max(t1, t2);
    
    float tMin = max(tMinVec.x, max(tMinVec.y, tMinVec.z));
    float tMax = min(tMaxVec.x, min(tMaxVec.y, tMaxVec.z));
    
    bool hit_success = (tMax >= tMin) && (tMax > 0.0);
    if (!hit_success) return false;
    
    hit.t = tMin > 0.0 ? tMin : tMax;
    
    vec3 hitPointLocal = rayOriginLocal + hit.t * ray.direction;
    hit.position = hitPointLocal + obj.position;
    
    vec3 distances = abs(hitPointLocal) - halfSize;
    
    const float epsilon = 1e-4;
    vec3 signs = sign(hitPointLocal);
    vec3 masks = step(abs(distances), vec3(epsilon));
    
    hit.normal = normalize(masks * signs);
    
    bool inside = all(lessThan(abs(rayOriginLocal), halfSize + vec3(epsilon)));
    hit.normal *= (inside ? -1.0 : 1.0);
    
    return true;
}


bool intersect(Ray ray, GPUObject obj, out hitInfo hit)
{
    if (obj.type == 0)
        return (intersectSphere(ray, obj, hit));
    if (obj.type == 1)
        return (intersectPlane(ray, obj, hit));
    if (obj.type == 2 || obj.type == 5)
        return (intersectQuad(ray, obj, hit));
    if (obj.type == 3)
        return (intersectTriangle(ray, obj, hit));
    if (obj.type == 4)
        return (intersectCube(ray, obj, hit));
    return (false);
}