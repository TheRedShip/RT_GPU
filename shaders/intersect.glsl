bool intersectSphere(Ray ray, GPUObject obj, out hitInfo hit)
{
	vec3 oc = ray.origin - obj.position;
	float b = dot(oc, ray.direction);
	float c = dot(oc, oc) - obj.radius * obj.radius;
	float discriminant = b * b - c;

	float sqrtD = sqrt(max(0.0, discriminant));
	float t0 = -b - sqrtD;
	float t1 = -b + sqrtD;

	float temp = min(t0, t1);
	t1 = max(t0, t1);
	t0 = temp;

	bool isInside = c < 0.0;
	t0 = isInside ? t1 : t0;

	hit.t = t0;
	hit.last_t = t1;
	hit.position = ray.origin + ray.direction * t0;
	hit.normal = normalize(hit.position - obj.position);
	hit.normal *= (isInside ? -1.0 : 1.0);

	return (discriminant >= 0.0) && (t0 > 0.0);
}

bool intersectPlane(Ray ray, GPUObject obj, out hitInfo hit)
{
	float d = dot(obj.normal, ray.direction);
	float t = dot(obj.position - ray.origin, obj.normal) / d;
	bool valid = t >= 0.0 && d != 0.0;

	// if (!valid) return (false);
	
	hit.t = t;
	hit.position = ray.origin + ray.direction * t;
	hit.normal = d < 0.0 ? obj.normal : -obj.normal;
	
	return (valid);
}

bool intersectQuad(Ray ray, GPUObject obj, out hitInfo hit)
{
	vec3 normal = obj.normal;
	float d = dot(normal, ray.direction);

	if (d == 0.0 || (obj.radius != 0.0 && d <= 0)) return (false); // double sided or not

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
	// hit.normal = normal;
	
	return (inside);
}

bool intersectTriangle(Ray ray, GPUTriangle obj, out hitInfo hit)
{
	vec3 vertex1 = obj.vertex1 - obj.position;
	vec3 vertex2 = obj.vertex2 - obj.position;
	
	vec3 pvec = cross(ray.direction, vertex2);
	float det = dot(vertex1, pvec);

	if (abs(det) < 1e-8) 
        return (false);

	vec3 tvec = ray.origin - obj.position;
	
	float invDet = 1.0 / det;
	hit.u = dot(tvec, pvec) * invDet;
	vec3 qvec = cross(tvec, vertex1);
	hit.v = dot(ray.direction, qvec) * invDet;
	hit.t = dot(vertex2, qvec) * invDet;
	
	bool valid = hit.u >= 0.0 && hit.u <= 1.0 &&
				hit.v >= 0.0 && (hit.u + hit.v) <= 1.0 &&
				hit.t > 0.0;
	
	// hit.position = ray.origin + ray.direction * t;
	// hit.normal = obj.normal * sign(-dot(ray.direction, obj.normal));
	// hit.normal = vec3(u, v, 1 - (u + v)); //texture mapping
	
	return (valid);
}

bool intersectTriangle(Ray ray, GPUObject obj, out hitInfo hit)
{
	GPUTriangle tri;

	tri.position = obj.position;
	tri.normal = obj.normal;
	tri.vertex1 = obj.vertex1;
	tri.vertex2 = obj.vertex2;

	return (intersectTriangle(ray, tri, hit));
}

bool intersectCube(Ray ray, GPUObject obj, out hitInfo hit)
{
	vec3 halfSize = obj.vertex1 * 0.5;
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

bool intersectCylinder(Ray ray, GPUObject obj, out hitInfo hit) 
{
	float radius = obj.normal.x;
	float height = obj.normal.y;
	
	vec3 rayOrigin = mat3(obj.rotation) * (ray.origin - obj.position);
    vec3 rayDir = mat3(obj.rotation) * ray.direction;
    
    float halfHeight = height * 0.5;
    float radius2 = radius * radius;
    
    vec2 oc_xz = rayOrigin.xz;
    vec2 rd_xz = rayDir.xz;
    float a = dot(rd_xz, rd_xz);
    float b = dot(oc_xz, rd_xz);
    float c = dot(oc_xz, oc_xz) - radius2;
    
    float h = b * b - a * c;
    if (h < 0.0) return (false);
    
    float t_cyl = (-b - sqrt(h)) / a;
    float y = rayOrigin.y + t_cyl * rayDir.y;
    
    t_cyl = mix((-b + sqrt(h)) / a, t_cyl, 
                float(abs(y) <= halfHeight && t_cyl > 0.0));
    y = rayOrigin.y + t_cyl * rayDir.y;
    
    float invRayDirY = 1.0 / rayDir.y;
    float t_cap = (-sign(rayDir.y) * halfHeight - rayOrigin.y) * invRayDirY;
    vec2 cap_xz = rayOrigin.xz + t_cap * rayDir.xz;
    bool cap_valid = (dot(cap_xz, cap_xz) <= radius2) && (t_cap > 0.0);
    
    bool cyl_valid = abs(y) <= halfHeight && t_cyl > 0.0;
    float t = mix(t_cap, t_cyl, float(cyl_valid && (t_cyl < t_cap || !cap_valid)));
    
    if (!cyl_valid && !cap_valid) return (false);
    
    vec3 p = rayOrigin + t * rayDir;
    
    vec3 n_side = normalize(vec3(p.x, 0.0, p.z));
    vec3 n_cap = vec3(0.0, -sign(rayDir.y), 0.0);
    vec3 normal = mix(n_cap, n_side, float(cyl_valid && (t_cyl < t_cap || !cap_valid)));
    
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    hit.normal = normalize(transpose(mat3(obj.rotation)) * normal);
    
    return (true);
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
	if (obj.type == 6) 
		return (intersectCylinder(ray, obj, hit));
	return (false);
}


bool intersectRayBVH(Ray ray, GPUBvh node, inout hitInfo hit)
{
	// vec3 inv_direction = 1.0 / ray.direction;

    vec3 t1 = (node.min - ray.origin) * ray.inv_direction;
    vec3 t2 = (node.max - ray.origin) * ray.inv_direction;
    
    vec3 tMin = min(t1, t2);
    vec3 tMax = max(t1, t2);
    
    hit.t = max(max(tMin.x, tMin.y), tMin.z);
    float last_t = min(min(tMax.x, tMax.y), tMax.z);
    
    return (hit.t <= last_t && last_t >= 0.0);
}


