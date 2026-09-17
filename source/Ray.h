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
