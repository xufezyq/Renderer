#pragma once

#include "Intersection.h"
#include "Ray.h"

class SceneObject;

class Primitive
{
public:
    virtual ~Primitive() = default;

    const SceneObject* GetSceneObject() const
    {
        return m_sceneObject;
    }

    virtual bool Intersect(
        const Ray& ray,
        Intersection& intersection) const = 0;

private:
    friend class SceneObject;

    void SetSceneObject(SceneObject* scene_object)
    {
        m_sceneObject = scene_object;
    }

    SceneObject* m_sceneObject = nullptr;
};
