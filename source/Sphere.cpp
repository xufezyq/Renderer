#include "Sphere.h"

#include <cmath>
#include <stdexcept>

Sphere::Sphere(float radius)
    : m_radius(radius)
{
    if (!std::isfinite(radius) || radius <= 0.0f)
        throw std::invalid_argument("Sphere radius must be finite and greater than zero");
}

bool Sphere::Intersect(const Ray& ray, Intersection& intersection) const
{
    const float a = glm::dot(ray.direction, ray.direction);
    const float b = 2.0f * glm::dot(ray.origin, ray.direction);
    const float c = glm::dot(ray.origin, ray.origin) - m_radius * m_radius;

    const float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
        return false;

    const float root = std::sqrt(discriminant);
    const float denominator = 2.0f * a;
    const float near_t = (-b - root) / denominator;
    const float far_t = (-b + root) / denominator;

    float hit_t = 0.0f;
    if (near_t >= ray.minT && near_t <= ray.maxT)
        hit_t = near_t;
    else if (far_t >= ray.minT && far_t <= ray.maxT)
        hit_t = far_t;
    else
        return false;

    const glm::vec3 local_position = ray.origin + hit_t * ray.direction;
    intersection.position = local_position;
    intersection.normal = glm::normalize(local_position / m_radius);
    intersection.t = hit_t;

    return true;
}
