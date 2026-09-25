#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

namespace
{
    glm::mat4 MakeObjectToWorld(const glm::vec3& position, const glm::vec3& euler, float scale)
    {
        if (!std::isfinite(scale) || scale <= 0.0f)
            throw std::invalid_argument("SceneObject scale must be finite and greater than zero");

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        transform = glm::rotate(transform, glm::radians(euler.z), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::rotate(transform, glm::radians(euler.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(euler.x), glm::vec3(1.0f, 0.0f, 0.0f));
        return glm::scale(transform, glm::vec3(scale));
    }
}

SceneObject::SceneObject(const glm::vec3& position, const glm::vec3& euler, float scale)
    : m_objectToWorld(MakeObjectToWorld(position, euler, scale)),
      m_worldToObject(glm::inverse(m_objectToWorld))
{}

SceneObject::~SceneObject() = default;

bool SceneObject::Intersect(const Ray& world_ray, Intersection& intersection) const
{
    const Ray local_ray = m_worldToObject * world_ray;
    Intersection closest_intersection{};
    float closest_t = local_ray.maxT;
    bool has_hit = false;

    for (const std::unique_ptr<Primitive>& primitive : m_primitives)
    {
        Intersection candidate{};
        if (!primitive->Intersect(local_ray, candidate) || candidate.t >= closest_t)
            continue;

        closest_t = candidate.t;
        closest_intersection = candidate;
        closest_intersection.primitive = primitive.get();
        has_hit = true;
    }

    if (!has_hit)
        return false;

    const glm::mat3 normal_matrix = glm::transpose(glm::mat3(m_worldToObject));
    intersection = closest_intersection;
    intersection.position = glm::vec3(m_objectToWorld * glm::vec4(closest_intersection.position, 1.0f));
    intersection.normal = glm::normalize(normal_matrix * closest_intersection.normal);
    return true;
}
