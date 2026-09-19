#pragma once

#include "Primitive.h"

#include <glm/glm.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class SceneObject
{
public:
    SceneObject(
        const glm::vec3& position,
        const glm::vec3& euler,
        float scale);

    ~SceneObject();

    template <typename T, typename... Args>
    T* CreatePrimitive(Args&&... args)
    {
        static_assert(
            std::is_base_of_v<Primitive, T>,
            "T must derive from Primitive");

        auto primitive = std::make_unique<T>(std::forward<Args>(args)...);
        T* primitive_pointer = primitive.get();
        primitive->SetSceneObject(this);
        m_primitives.emplace_back(std::move(primitive));
        return primitive_pointer;
    }

    bool Intersect(
        const Ray& world_ray,
        Intersection& intersection) const;

private:
    glm::mat4 m_objectToWorld;
    glm::mat4 m_worldToObject;
    std::vector<std::unique_ptr<Primitive>> m_primitives;
};
