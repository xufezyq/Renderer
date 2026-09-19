#include "Renderer.h"

#include <cassert>
#include <cmath>

namespace
{
    void assert_near(float actual, float expected, float tolerance = 1.0e-3f)
    {
        assert(std::fabs(actual - expected) < tolerance);
    }
}

int main()
{
    const Renderer renderer(800, 600);

    const Color sphere_color = renderer.RednerPixel(500, 300);
    assert(sphere_color.r > 0.65f);
    assert_near(sphere_color.g, 0.5f);
    assert(sphere_color.b > 0.02f);

    const Color corner_color = renderer.RednerPixel(0, 0);
    assert_near(corner_color.r, 0.0f);
    assert_near(corner_color.g, 0.0f);
    assert_near(corner_color.b, 0.0f);

    return 0;
}
