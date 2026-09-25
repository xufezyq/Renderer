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

    void assert_color_near(const Color& actual, const Color& expected)
    {
        assert_near(actual.r, expected.r);
        assert_near(actual.g, expected.g);
        assert_near(actual.b, expected.b);
    }
}

int main()
{
    const Renderer renderer(800, 600, false);
    const Ray top_ray{glm::vec3(0.0f, 2.0f, 3.0f), glm::vec3(0.0f, -1.0f, 0.0f)};
    const float distance = std::sqrt(1.3f * 1.3f + 2.0f * 2.0f);
    const float point_attenuation = 1.0f / (1.0f + 0.5f * distance + 0.25f * distance * distance);
    const Color top_irradiance = renderer.GetIrradiance(top_ray);
    assert_color_near(top_irradiance, Color{2.0f + 4.0f * point_attenuation * 1.3f / distance, 2.0f + 2.0f * point_attenuation * 1.3f / distance, 2.0f + point_attenuation * 1.3f / distance});

    const Ray bottom_ray{glm::vec3(0.0f, -2.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
    const Color bottom_irradiance = renderer.GetIrradiance(bottom_ray);
    assert_color_near(bottom_irradiance, Color{0.0f, 0.0f, 0.0f});

    const Ray front_ray{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    const Color front_irradiance = renderer.GetIrradiance(front_ray);
    const float spot_contribution = 3.0f / (1.0f + 0.2f * 2.3f + 0.1f * 2.3f * 2.3f);
    assert_color_near(front_irradiance, Color{4.0f * point_attenuation * 1.3f / distance + spot_contribution, 2.0f * point_attenuation * 1.3f / distance + spot_contribution, point_attenuation * 1.3f / distance + spot_contribution});

    const Ray miss_ray{glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)};
    assert_color_near(renderer.GetIrradiance(miss_ray), Color{0.0f, 0.0f, 0.0f});

    Scene scene;
    scene.LoadSceneFromXML("scene.xml");
    Camera camera;
    const SceneCameraSettings& settings = scene.GetCameraSettings();
    camera.Initialize(settings.position, settings.forward, settings.up, settings.verticalFov, 0.1f, 1000.0f, 800, 600);
    const Color center_pixel = renderer.RednerPixel(400, 300);
    assert_color_near(center_pixel, renderer.GetIrradiance(camera.GetRay(400.5f, 300.5f)));
    assert(center_pixel.r > 0.0f);

    const Renderer renderer_with_one_sample(800, 600, true, 1);
    assert_color_near(renderer_with_one_sample.RednerPixel(400, 300), center_pixel);

    const Renderer renderer_with_ssaa(800, 600, true, 2);
    Color expected_ssaa{};
    for (unsigned sample_y = 0; sample_y < 2; ++sample_y)
    {
        for (unsigned sample_x = 0; sample_x < 2; ++sample_x)
        {
            const Color sample = renderer.GetIrradiance(camera.GetRay(400.0f + (static_cast<float>(sample_x) + 0.5f) * 0.5f, 300.0f + (static_cast<float>(sample_y) + 0.5f) * 0.5f));
            expected_ssaa.r += sample.r * 0.25f;
            expected_ssaa.g += sample.g * 0.25f;
            expected_ssaa.b += sample.b * 0.25f;
        }
    }
    assert_color_near(renderer_with_ssaa.RednerPixel(400, 300), expected_ssaa);

    const Color background = renderer.RednerPixel(0, 0);
    assert_color_near(background, Color{0.0f, 0.0f, 0.0f});

    const Renderer cornell_renderer(800, 600, true, 2, "cornell_box.xml");
    const Color cornell_center = cornell_renderer.RednerPixel(400, 300);
    assert(std::isfinite(cornell_center.r) && std::isfinite(cornell_center.g) && std::isfinite(cornell_center.b));
    assert(cornell_center.r > 0.0f || cornell_center.g > 0.0f || cornell_center.b > 0.0f);

    const Renderer shadow_renderer(800, 600, false, 1, "ShadowScene.xml");
    const Ray shadowed_ray{glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, 1.0f)};
    const Color shadowed_color = shadow_renderer.GetIrradiance(shadowed_ray);
    assert_color_near(shadowed_color, Color{0.0f, 0.0f, 0.3f});

    const Ray lit_ray{glm::vec3(0.8f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, 1.0f)};
    const Color lit_color = shadow_renderer.GetIrradiance(lit_ray);
    assert(lit_color.r > 0.0f && lit_color.g > 0.0f && lit_color.b > 0.0f);

    const Renderer directional_shadow_renderer(800, 600, false, 1, "DirectionalShadowScene.xml");
    const Ray blocked_directional_ray{glm::vec3(2.0f, 0.0f, 3.4f), glm::vec3(-1.0f, 0.0f, 0.0f)};
    assert_color_near(directional_shadow_renderer.GetIrradiance(blocked_directional_ray), Color{0.0f, 0.0f, 0.0f});
    const Ray unblocked_directional_ray{glm::vec3(-2.0f, 0.0f, 3.4f), glm::vec3(1.0f, 0.0f, 0.0f)};
    assert_color_near(directional_shadow_renderer.GetIrradiance(unblocked_directional_ray), Color{0.3f, 0.3f, 0.3f});

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

    return 0;
}
