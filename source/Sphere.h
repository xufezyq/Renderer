#pragma once

#include "Intersection.h"
#include "Primitive.h"
#include "Ray.h"

class Sphere final : public Primitive
{
public:
    explicit Sphere(float radius);

    // 在球体的局部坐标系中返回射线范围内最近的交点信息。
    bool Intersect(const Ray& ray, Intersection& intersection) const override;

private:
    float m_radius;
};
