#include "Triangle.h"

#include <cmath>
#include <stdexcept>

namespace
{
    constexpr float intersection_epsilon = 1.0e-6f;
}

Triangle::Triangle(
    const glm::vec3& vertex0,
    const glm::vec3& vertex1,
    const glm::vec3& vertex2)
    : m_vertex0(vertex0),
      m_vertex1(vertex1),
      m_vertex2(vertex2),
      m_normal(0.0f)
{
    const glm::vec3 unnormalized_normal = glm::cross(m_vertex1 - m_vertex0, m_vertex2 - m_vertex0);
    const float normal_length_squared = glm::dot(unnormalized_normal, unnormalized_normal);
    if (!std::isfinite(normal_length_squared) || normal_length_squared <= intersection_epsilon * intersection_epsilon)
        throw std::invalid_argument("Triangle vertices must form a non-degenerate triangle");

    m_normal = glm::normalize(unnormalized_normal);
}

bool Triangle::Intersect(const Ray& ray, Intersection& intersection) const
{
    const glm::vec3 edge1 = m_vertex1 - m_vertex0;
    const glm::vec3 edge2 = m_vertex2 - m_vertex0;
    const glm::vec3 direction_cross_edge2 = glm::cross(ray.direction, edge2);
    const float determinant = glm::dot(edge1, direction_cross_edge2);
    if (std::fabs(determinant) < intersection_epsilon)
        return false;

    const float inverse_determinant = 1.0f / determinant;
    const glm::vec3 vertex0_to_origin = ray.origin - m_vertex0;
    const float u = glm::dot(vertex0_to_origin, direction_cross_edge2) * inverse_determinant;
    if (u < 0.0f || u > 1.0f)
        return false;

    const glm::vec3 origin_cross_edge1 = glm::cross(vertex0_to_origin, edge1);
    const float v = glm::dot(ray.direction, origin_cross_edge1) * inverse_determinant;
    if (v < 0.0f || u + v > 1.0f)
        return false;

    const float hit_t = glm::dot(edge2, origin_cross_edge1) * inverse_determinant;
    if (hit_t < ray.minT || hit_t > ray.maxT)
        return false;

    intersection.position = ray.origin + hit_t * ray.direction;
    intersection.normal = m_normal;
    intersection.t = hit_t;

    return true;
}
