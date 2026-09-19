#pragma once

#include "Intersection.h"
#include "Ray.h"

class Primitive
{
public:
    virtual ~Primitive() = default;

    virtual bool Intersect(
        const Ray& ray,
        Intersection& intersection) const = 0;
};
