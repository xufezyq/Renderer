#pragma once

#include "SceneObject.h"

#include <memory>
#include <string>
#include <vector>

class Scene
{
public:
    ~Scene();

    void LoadSceneFromXML(const std::string& file_path);

    SceneObject* CreateSceneObject(
        const glm::vec3& position,
        const glm::vec3& euler,
        float scale);

    bool Intersect(
        const Ray& ray,
        Intersection& intersection) const;

private:
    std::vector<std::unique_ptr<SceneObject>> m_sceneObjects;
};
