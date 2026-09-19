#include "Triangle.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

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
    const Triangle triangle(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    Intersection intersection{};
    const Ray front_ray{glm::vec3(0.25f, 0.25f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(triangle.Intersect(front_ray, intersection));
    assert_near(intersection.t, 1.0f);
    assert_vector_near(intersection.position, glm::vec3(0.25f, 0.25f, 0.0f));
    assert_vector_near(intersection.normal, glm::vec3(0.0f, 0.0f, 1.0f));

    const Ray back_ray{glm::vec3(0.25f, 0.25f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)};
    assert(triangle.Intersect(back_ray, intersection));
    assert_near(intersection.t, 1.0f);

    const Ray outside_ray{glm::vec3(0.75f, 0.75f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(!triangle.Intersect(outside_ray, intersection));

    const Ray parallel_ray{glm::vec3(0.25f, 0.25f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)};
    assert(!triangle.Intersect(parallel_ray, intersection));

    Ray clipped_ray{glm::vec3(0.25f, 0.25f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    clipped_ray.maxT = 0.5f;
    assert(!triangle.Intersect(clipped_ray, intersection));

    bool rejected_degenerate_triangle = false;
    try
    {
        const Triangle degenerate(
            glm::vec3(0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 0.0f, 0.0f));
        (void)degenerate;
    }
    catch (const std::invalid_argument&)
    {
        rejected_degenerate_triangle = true;
    }
    assert(rejected_degenerate_triangle);

    return 0;
}
