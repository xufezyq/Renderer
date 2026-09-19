#pragma once

#include "Intersection.h"
#include "Primitive.h"
#include "Ray.h"

#include <glm/glm.hpp>

class Sphere final : public Primitive
{
public:
    Sphere(const glm::vec3& center, float radius);

    // 返回射线范围内最近的世界空间交点信息。
    bool Intersect(const Ray& ray, Intersection& intersection) const override;

private:
    float m_radius;
    glm::mat4 m_objectToWorld;
    glm::mat4 m_worldToObject;
};
