#pragma once

#include "Light.h"
#include "SceneObject.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

struct SceneCameraSettings
{
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 forward{0.0f, 0.0f, 1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float verticalFov = 60.0f;
};

class Scene
{
public:
    ~Scene();

    void LoadSceneFromXML(const std::string& file_path);
    const SceneCameraSettings& GetCameraSettings() const;

    SceneObject* CreateSceneObject(const glm::vec3& position, const glm::vec3& euler, float scale);

    // 场景持有创建的光源；返回的指针在场景销毁或重新加载前有效。
    template <typename T, typename... Args>
    T* CreateLight(Args&&... args)
    {
        static_assert(std::is_base_of_v<Light, T>, "T must derive from Light");
        auto light = std::make_unique<T>(std::forward<Args>(args)...);
        T* light_pointer = light.get();
        m_lights.emplace_back(std::move(light));
        return light_pointer;
    }

    const std::vector<std::unique_ptr<Light>>& GetLights() const;

    bool Intersect(const Ray& ray, Intersection& intersection) const;

private:
    SceneCameraSettings m_cameraSettings;
    std::vector<std::unique_ptr<SceneObject>> m_sceneObjects;
    std::vector<std::unique_ptr<Light>> m_lights;
};
