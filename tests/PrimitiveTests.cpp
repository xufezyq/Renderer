#include "Primitive.h"

#include <cassert>

namespace
{
    class TestPrimitive final : public Primitive
    {
    public:
        bool Intersect(const Ray& ray, Intersection& intersection) const override
        {
            intersection.t = ray.minT;
            return true;
        }
    };
}

int main()
{
    const Primitive& primitive = TestPrimitive{};
    Intersection intersection{};
    const Ray ray{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};

    assert(primitive.Intersect(ray, intersection));
    assert(intersection.t == ray.minT);
    return 0;
}
