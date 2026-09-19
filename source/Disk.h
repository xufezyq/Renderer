#pragma once

#include "Intersection.h"
#include "Ray.h"

#include <glm/glm.hpp>

class Disk
{
public:
    Disk(const glm::vec3& center, float radius);

    // 圆盘位于局部 XY 平面，法线朝向局部 +Z。
    bool Intersect(const Ray& ray, Intersection& intersection) const;

private:
    float m_radius;
    glm::mat4 m_objectToWorld;
    glm::mat4 m_worldToObject;
};
