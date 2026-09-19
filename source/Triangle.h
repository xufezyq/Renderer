#pragma once

#include "Intersection.h"
#include "Ray.h"

#include <glm/glm.hpp>

class Triangle
{
public:
    Triangle(
        const glm::vec3& vertex0,
        const glm::vec3& vertex1,
        const glm::vec3& vertex2);

    // 使用 Moller-Trumbore 算法进行双面射线求交。
    bool Intersect(const Ray& ray, Intersection& intersection) const;

private:
    glm::vec3 m_vertex0;
    glm::vec3 m_vertex1;
    glm::vec3 m_vertex2;
    glm::vec3 m_normal;
};
