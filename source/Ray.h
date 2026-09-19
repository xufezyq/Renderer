#pragma once

#include <glm/glm.hpp>

#include <limits>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    float minT = 0.001f;
    float maxT = std::numeric_limits<float>::infinity();
};

inline Ray operator*(const glm::mat4& matrix, const Ray& ray)
{
    Ray transformed_ray;

    transformed_ray.origin = glm::vec3(matrix * glm::vec4(ray.origin, 1.0f));
    transformed_ray.direction = glm::vec3(matrix * glm::vec4(ray.direction, 0.0f));
    transformed_ray.minT = ray.minT;
    transformed_ray.maxT = ray.maxT;

    return transformed_ray;
}