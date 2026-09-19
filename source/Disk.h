#pragma once

#include "Intersection.h"
#include "Primitive.h"
#include "Ray.h"

class Disk final : public Primitive
{
public:
    explicit Disk(float radius);

    // 圆盘位于局部 XY 平面，法线朝向局部 +Z。
    bool Intersect(const Ray& ray, Intersection& intersection) const override;

private:
    float m_radius;
};
