#include "Scene.h"
#include "Sphere.h"

#include <cassert>
#include <cmath>

int main()
{
    Scene scene;
    SceneObject* object = scene.CreateSceneObject(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), 1.0f);
    assert(object != nullptr);

    object->CreatePrimitive<Sphere>(1.0f);

    Intersection intersection{};
    const Ray ray{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(scene.Intersect(ray, intersection));
    assert(intersection.t > 1.9f && intersection.t < 2.1f);
    assert(intersection.primitive != nullptr);
    assert(intersection.primitive->GetSceneObject() == object);

    PointLight* point_light = scene.CreateLight<PointLight>(glm::vec3(0.0f, 2.0f, 0.0f), Color{4.0f, 2.0f, 1.0f}, glm::vec3(0.0f, 0.0f, 1.0f));
    assert(point_light != nullptr);
    assert(scene.GetLights().size() == 1);
    assert(scene.GetLights().front().get() == point_light);
    glm::vec3 source_pos(0.0f);
    const Color light_color = scene.GetLights().front()->GetRadiance(glm::vec3(0.0f), source_pos);
    assert(source_pos == glm::vec3(0.0f, 2.0f, 0.0f));
    assert(std::fabs(light_color.r - 4.0f) < 1.0e-5f);

    DirectionalLight* directional_light = scene.CreateLight<DirectionalLight>(glm::vec3(0.0f, -1.0f, 0.0f), Color{1.0f, 1.0f, 1.0f});
    SpotLight* spot_light = scene.CreateLight<SpotLight>(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), Color{1.0f, 1.0f, 1.0f}, 20.0f, 40.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    assert(scene.GetLights().size() == 3);
    assert(scene.GetLights()[0].get() == point_light);
    assert(scene.GetLights()[1].get() == directional_light);
    assert(scene.GetLights()[2].get() == spot_light);

    scene.LoadSceneFromXML("scene.xml");
    assert(scene.GetLights().size() == 3);
    const auto* loaded_directional = dynamic_cast<const DirectionalLight*>(scene.GetLights()[0].get());
    const auto* loaded_point = dynamic_cast<const PointLight*>(scene.GetLights()[1].get());
    const auto* loaded_spot = dynamic_cast<const SpotLight*>(scene.GetLights()[2].get());
    assert(loaded_directional != nullptr && loaded_directional->GetDirection() == glm::vec3(0.0f, -1.0f, 0.0f));
    const Color loaded_radiance = loaded_directional->GetRadiance(glm::vec3(0.0f), source_pos);
    assert(loaded_radiance.r == 2.0f && loaded_radiance.g == 2.0f && loaded_radiance.b == 2.0f);
    assert(loaded_point != nullptr && loaded_point->GetPosition() == glm::vec3(0.0f, 2.0f, 1.0f));
    assert(loaded_point->GetIntensity().r == 4.0f && loaded_point->GetIntensity().g == 2.0f && loaded_point->GetIntensity().b == 1.0f);
    assert(loaded_point->GetAttenuations() == glm::vec3(0.25f, 0.5f, 1.0f));
    assert(loaded_spot != nullptr && loaded_spot->GetDirection() == glm::vec3(0.0f, 0.0f, 1.0f));
    assert(loaded_spot->GetPosition() == glm::vec3(0.0f));
    assert(loaded_spot->GetIntensity().r == 3.0f && loaded_spot->GetIntensity().g == 3.0f && loaded_spot->GetIntensity().b == 3.0f);
    assert(loaded_spot->GetInnerAngleDegrees() == 20.0f && loaded_spot->GetOuterAngleDegrees() == 40.0f);
    assert(loaded_spot->GetAttenuations() == glm::vec3(0.1f, 0.2f, 1.0f));

    Scene loaded_scene;
    loaded_scene.LoadSceneFromXML("scene.xml");
    Intersection loaded_intersection{};
    assert(loaded_scene.Intersect(ray, loaded_intersection));
    assert(loaded_intersection.primitive != nullptr);

    Scene cornell_scene;
    cornell_scene.LoadSceneFromXML("cornell_box.xml");
    assert(cornell_scene.GetLights().size() == 2);
    assert(dynamic_cast<const PointLight*>(cornell_scene.GetLights()[0].get()) != nullptr);
    const auto* cornell_spot = dynamic_cast<const SpotLight*>(cornell_scene.GetLights()[1].get());
    assert(cornell_spot != nullptr);
    assert(cornell_spot->GetPosition() == glm::vec3(-1.5f, 1.5f, 4.6f));
    assert(glm::dot(cornell_spot->GetDirection(), glm::normalize(glm::vec3(1.4f, -1.9f, 0.0f))) > 0.999f);
    Intersection cornell_intersection{};
    assert(cornell_scene.Intersect(ray, cornell_intersection));
    assert(cornell_intersection.primitive != nullptr);

    Scene dust2_scene;
    dust2_scene.LoadSceneFromXML("dust2.xml");
    assert(dust2_scene.GetCameraSettings().position == glm::vec3(0.0f, 36.0f, -4.0f));
    assert(dust2_scene.GetCameraSettings().verticalFov == 75.0f);

    Scene legacy_scene;
    legacy_scene.LoadSceneFromXML("dust2_model.xml");
    assert(legacy_scene.GetCameraSettings().position == glm::vec3(-18.782f, 3.773f, -9.283f));
    assert(legacy_scene.GetCameraSettings().verticalFov == 65.0f);

    return 0;
}
