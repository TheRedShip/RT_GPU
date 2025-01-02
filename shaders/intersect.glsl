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
    
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    hit.normal = d < 0.0 ? obj.normal : -obj.normal;
    
    return (valid);
}

bool intersect(Ray ray, GPUObject obj, out hitInfo hit)
{
    if (obj.type == 0)
        return (intersectSphere(ray, obj, hit));
    if (obj.type == 1)
        return (intersectPlane(ray, obj, hit));
    return (false);
}