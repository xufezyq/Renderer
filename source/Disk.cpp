#include "Disk.h"

#include <cmath>
#include <stdexcept>

Disk::Disk(float radius)
    : m_radius(radius)
{
    if (!std::isfinite(radius) || radius <= 0.0f)
        throw std::invalid_argument("Disk radius must be finite and greater than zero");
}

bool Disk::Intersect(const Ray& ray, Intersection& intersection) const
{
    if (std::fabs(ray.direction.z) < 1.0e-6f)
        return false;

    const float hit_t = -ray.origin.z / ray.direction.z;
    if (hit_t < ray.minT || hit_t > ray.maxT)
        return false;

    const glm::vec3 local_position = ray.origin + hit_t * ray.direction;
    const float distance_squared = local_position.x * local_position.x +
                                   local_position.y * local_position.y;
    if (distance_squared > m_radius * m_radius)
        return false;

    intersection.position = local_position;
    intersection.normal = glm::vec3(0.0f, 0.0f, 1.0f);
    intersection.t = hit_t;

    return true;
}
