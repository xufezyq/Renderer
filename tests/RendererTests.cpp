#include "Renderer.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

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

    const Color sphere_color = renderer.RednerPixel(400, 300);
    assert_near(sphere_color.r, 0.5f, 0.01f);
    assert_near(sphere_color.g, 0.5f, 0.01f);
    assert_near(sphere_color.b, 0.0f, 0.01f);

    const Color sphere_edge_color = renderer.RednerPixel(510, 300);
    assert(sphere_edge_color.r > 0.7f);
    assert(sphere_edge_color.b > 0.05f);

    const Color disk_color = renderer.RednerPixel(780, 300);
    assert_near(disk_color.r, 0.5f);
    assert_near(disk_color.g, 0.5f);
    assert_near(disk_color.b, 1.0f);

    const Color antialiased_edge = renderer.RednerPixel(789, 300);
    assert(antialiased_edge.r > 0.0f);
    assert(antialiased_edge.r < 0.5f);
    assert(antialiased_edge.g > 0.0f);
    assert(antialiased_edge.g < 0.5f);
    assert(antialiased_edge.b > 0.0f);
    assert(antialiased_edge.b < 1.0f);

    const Renderer renderer_without_ssaa(800, 600, false);
    const Color aliased_edge = renderer_without_ssaa.RednerPixel(789, 300);
    assert_near(aliased_edge.r, 0.5f);
    assert_near(aliased_edge.g, 0.5f);
    assert_near(aliased_edge.b, 1.0f);

    const Renderer renderer_with_one_sample(800, 600, true, 1);
    const Color one_sample_edge = renderer_with_one_sample.RednerPixel(789, 300);
    assert_near(one_sample_edge.r, 0.5f);
    assert_near(one_sample_edge.g, 0.5f);
    assert_near(one_sample_edge.b, 1.0f);

    bool rejected_zero_samples = false;
    try
    {
        const Renderer invalid_renderer(800, 600, true, 0);
        (void)invalid_renderer;
    }
    catch (const std::invalid_argument&)
    {
        rejected_zero_samples = true;
    }
    assert(rejected_zero_samples);

    const Color triangle_color = renderer.RednerPixel(62, 340);
    assert_near(triangle_color.r, 0.5f);
    assert_near(triangle_color.g, 0.5f);
    assert_near(triangle_color.b, 1.0f);

    const Color gap_color = renderer.RednerPixel(536, 300);
    assert_near(gap_color.r, 0.0f);
    assert_near(gap_color.g, 0.0f);
    assert_near(gap_color.b, 0.0f);

    const Renderer cornell_renderer(800, 600, true, 2, "cornell_box.xml");
    const Color cornell_center = cornell_renderer.RednerPixel(400, 300);
    assert_near(cornell_center.r, 0.5f);
    assert_near(cornell_center.g, 0.5f);
    assert_near(cornell_center.b, 0.0f);

    const Color cornell_ceiling = cornell_renderer.RednerPixel(400, 100);
    assert_near(cornell_ceiling.r, 0.5f);
    assert_near(cornell_ceiling.g, 0.0f);
    assert_near(cornell_ceiling.b, 0.5f);

    const Color cornell_floor = cornell_renderer.RednerPixel(400, 500);
    assert_near(cornell_floor.r, 0.5f);
    assert_near(cornell_floor.g, 1.0f);
    assert_near(cornell_floor.b, 0.5f);

    const Color cornell_left_wall = cornell_renderer.RednerPixel(100, 300);
    assert_near(cornell_left_wall.r, 1.0f);
    assert_near(cornell_left_wall.g, 0.5f);
    assert_near(cornell_left_wall.b, 0.5f);

    const Color cornell_right_wall = cornell_renderer.RednerPixel(700, 300);
    assert_near(cornell_right_wall.r, 0.0f);
    assert_near(cornell_right_wall.g, 0.5f);
    assert_near(cornell_right_wall.b, 0.5f);

    const Color corner_color = renderer.RednerPixel(0, 0);
    assert_near(corner_color.r, 0.0f);
    assert_near(corner_color.g, 0.0f);
    assert_near(corner_color.b, 0.0f);

    return 0;
}
