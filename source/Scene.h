#pragma once

#include "SceneObject.h"

#include <memory>
#include <string>
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

    SceneObject* CreateSceneObject(
        const glm::vec3& position,
        const glm::vec3& euler,
        float scale);

    bool Intersect(
        const Ray& ray,
        Intersection& intersection) const;

private:
    SceneCameraSettings m_cameraSettings;
    std::vector<std::unique_ptr<SceneObject>> m_sceneObjects;
};
