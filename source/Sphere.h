#pragma once

#include "Ray.h"

#include <glm/glm.hpp>

struct Intersection
{
    glm::vec3 position;
    glm::vec3 normal;
    float t;
};

class Sphere
{
public:
    Sphere(const glm::vec3& center, float radius);

    // 返回射线范围内最近的世界空间交点信息。
    bool Intersect(const Ray& ray, Intersection& intersection) const;

private:
    float m_radius;
    glm::mat4 m_objectToWorld;
    glm::mat4 m_worldToObject;
};
