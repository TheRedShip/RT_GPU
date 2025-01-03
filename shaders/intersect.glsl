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

bool intersectQuad(Ray ray, GPUObject obj, out hitInfo hit)
{
    // obj.edge1 and obj.edge2 are the two edge vectors from quad origin
    vec3 normal = normalize(cross(obj.edge1, obj.edge2));
    float d = dot(normal, ray.direction);
    float t = dot(obj.position - ray.origin, normal) / d;

    if (t <= 0.0 || d == 0.0) return (false);
    
    // Get hit point relative to quad origin
    vec3 p = ray.origin + ray.direction * t - obj.position;
    
    // Project hit point onto edges using dot product
    float e1 = dot(p, obj.edge1);
    float e2 = dot(p, obj.edge2);
    
    // Check if point is inside quad using edge lengths
    float l1 = dot(obj.edge1, obj.edge1);
    float l2 = dot(obj.edge2, obj.edge2);
    
    bool inside = e1 >= 0.0 && e1 <= l1 && e2 >= 0.0 && e2 <= l2;
    
    hit.t = t;
    hit.position = p + obj.position;
    hit.normal = d < 0.0 ? normal : -normal;
    
    return (inside);
}

bool intersect(Ray ray, GPUObject obj, out hitInfo hit)
{
    if (obj.type == 0)
        return (intersectSphere(ray, obj, hit));
    if (obj.type == 1)
        return (intersectPlane(ray, obj, hit));
    if (obj.type == 2)
        return (intersectQuad(ray, obj, hit));

    return (false);
}