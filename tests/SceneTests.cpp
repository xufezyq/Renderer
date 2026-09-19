#include "Scene.h"
#include "Sphere.h"

#include <cassert>
int main()
{
    Scene scene;
    SceneObject* object = scene.CreateSceneObject(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f),
        1.0f);
    assert(object != nullptr);

    object->CreatePrimitive<Sphere>(1.0f);

    Intersection intersection{};
    const Ray ray{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    assert(scene.Intersect(ray, intersection));
    assert(intersection.t > 1.9f && intersection.t < 2.1f);
    assert(intersection.primitive != nullptr);
    assert(intersection.primitive->GetSceneObject() == object);

    Scene loaded_scene;
    loaded_scene.LoadSceneFromXML("scene.xml");
    Intersection loaded_intersection{};
    assert(loaded_scene.Intersect(ray, loaded_intersection));
    assert(loaded_intersection.primitive != nullptr);

    Scene cornell_scene;
    cornell_scene.LoadSceneFromXML("cornell_box.xml");
    Intersection cornell_intersection{};
    assert(cornell_scene.Intersect(ray, cornell_intersection));
    assert(cornell_intersection.primitive != nullptr);

    return 0;
}
