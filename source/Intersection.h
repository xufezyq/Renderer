#pragma once

#include <glm/glm.hpp>

class Primitive;

struct Intersection
{
    glm::vec3 position;
    glm::vec3 normal;
    float t;
    const Primitive* primitive = nullptr;
};
