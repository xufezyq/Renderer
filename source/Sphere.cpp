#include "Sphere.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

Sphere::Sphere(const glm::vec3& center, float radius)
    : m_radius(radius),
      m_objectToWorld(glm::translate(glm::mat4(1.0f), center)),
      m_worldToObject(glm::translate(glm::mat4(1.0f), -center))
{
    if (!std::isfinite(radius) || radius <= 0.0f)
        throw std::invalid_argument("Sphere radius must be finite and greater than zero");
}

bool Sphere::Intersect(const Ray& ray, Intersection& intersection) const
{
    const Ray local_ray = m_worldToObject * ray;

    const float a = glm::dot(local_ray.direction, local_ray.direction);
    const float b = 2.0f * glm::dot(local_ray.origin, local_ray.direction);
    const float c = glm::dot(local_ray.origin, local_ray.origin) - m_radius * m_radius;

    const float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
        return false;

    const float root = std::sqrt(discriminant);
    const float denominator = 2.0f * a;
    const float near_t = (-b - root) / denominator;
    const float far_t = (-b + root) / denominator;

    float hit_t = 0.0f;
    if (near_t >= local_ray.minT && near_t <= local_ray.maxT)
        hit_t = near_t;
    else if (far_t >= local_ray.minT && far_t <= local_ray.maxT)
        hit_t = far_t;
    else
        return false;

    const glm::vec3 local_position = local_ray.origin + hit_t * local_ray.direction;
    const glm::vec3 local_normal = local_position / m_radius;
    const glm::mat3 normal_matrix = glm::transpose(glm::mat3(m_worldToObject));

    intersection.position = glm::vec3(m_objectToWorld * glm::vec4(local_position, 1.0f));
    intersection.normal = glm::normalize(normal_matrix * local_normal);
    intersection.t = hit_t;

    return true;
}
