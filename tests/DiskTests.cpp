#include "Disk.h"

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
    const Disk disk(glm::vec3(1.0f, 2.0f, 3.0f), 2.0f);

    Intersection intersection{};
    const Ray center_ray{glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(disk.Intersect(center_ray, intersection));
    assert_near(intersection.t, 3.0f);
    assert_vector_near(intersection.position, glm::vec3(1.0f, 2.0f, 3.0f));
    assert_vector_near(intersection.normal, glm::vec3(0.0f, 0.0f, 1.0f));

    const Ray outside_ray{glm::vec3(3.1f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(!disk.Intersect(outside_ray, intersection));

    const Ray parallel_ray{glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};
    assert(!disk.Intersect(parallel_ray, intersection));

    Ray clipped_ray{glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    clipped_ray.maxT = 2.0f;
    assert(!disk.Intersect(clipped_ray, intersection));

    return 0;
}
