#include "Disk.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

Disk::Disk(const glm::vec3& center, float radius)
    : m_radius(radius),
      m_objectToWorld(glm::translate(glm::mat4(1.0f), center)),
      m_worldToObject(glm::translate(glm::mat4(1.0f), -center))
{
    if (!std::isfinite(radius) || radius <= 0.0f)
        throw std::invalid_argument("Disk radius must be finite and greater than zero");
}

bool Disk::Intersect(const Ray& ray, Intersection& intersection) const
{
    const Ray local_ray = m_worldToObject * ray;
    if (std::fabs(local_ray.direction.z) < 1.0e-6f)
        return false;

    const float hit_t = -local_ray.origin.z / local_ray.direction.z;
    if (hit_t < local_ray.minT || hit_t > local_ray.maxT)
        return false;

    const glm::vec3 local_position = local_ray.origin + hit_t * local_ray.direction;
    const float distance_squared = local_position.x * local_position.x +
                                   local_position.y * local_position.y;
    if (distance_squared > m_radius * m_radius)
        return false;

    const glm::vec3 local_normal(0.0f, 0.0f, 1.0f);
    const glm::mat3 normal_matrix = glm::transpose(glm::mat3(m_worldToObject));

    intersection.position = glm::vec3(m_objectToWorld * glm::vec4(local_position, 1.0f));
    intersection.normal = glm::normalize(normal_matrix * local_normal);
    intersection.t = hit_t;

    return true;
}
