#include "Sphere.h"

#include <cassert>
#include <cmath>

namespace
{
    void assert_near(float actual, float expected)
    {
        assert(std::fabs(actual - expected) < 1.0e-5f);
    }

    void assert_vector_near(const glm::vec3& actual, const glm::vec3& expected)
    {
        assert_near(actual.x, expected.x);
        assert_near(actual.y, expected.y);
        assert_near(actual.z, expected.z);
    }
}

int main()
{
    const Sphere sphere(1.0f);

    Intersection intersection{};
    const Ray front_ray{glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(sphere.Intersect(front_ray, intersection));
    assert_near(intersection.t, 2.0f);
    assert_vector_near(intersection.position, glm::vec3(0.0f, 0.0f, -1.0f));
    assert_vector_near(intersection.normal, glm::vec3(0.0f, 0.0f, -1.0f));

    const Ray inside_ray{glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};
    assert(sphere.Intersect(inside_ray, intersection));
    assert_near(intersection.t, 1.0f);
    assert_vector_near(intersection.position, glm::vec3(1.0f, 0.0f, 0.0f));
    assert_vector_near(intersection.normal, glm::vec3(1.0f, 0.0f, 0.0f));

    const Ray miss_ray{glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
    assert(!sphere.Intersect(miss_ray, intersection));

    Ray clipped_ray{glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    clipped_ray.maxT = 1.5f;
    assert(!sphere.Intersect(clipped_ray, intersection));

    return 0;
}
