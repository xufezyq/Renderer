#pragma once

#include "Ray.h"

#include <glm/glm.hpp>

class Camera
{
public:
    // 根据相机姿态、透视参数和渲染尺寸创建各级变换矩阵。
    // position 是世界坐标；forward 和 up 指定朝向；verticalFov 单位为度。
    // nearClip、farClip 是裁剪距离；width、height 是渲染区域尺寸。
    void Initialize(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, float verticalFov, float nearClip, float farClip, unsigned width, unsigned height);

    // 以下接口分别提供观察、投影、视口及三者的联合矩阵。
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;
    const glm::mat4& GetViewportMatrix() const;
    const glm::mat4& GetCombinedMatrix() const;
    const glm::mat4& GetInverseCombinedMatrix() const;

    // 根据屏幕像素坐标生成一条从摄像机位置出发的世界空间射线。
    Ray GetRay(float x, float y) const;

private:
    // 使用左手坐标系，投影后的 NDC 深度范围为 [0, 1]。
    glm::vec3 m_position{0.0f};
    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
    glm::mat4 m_viewportMatrix{1.0f};
    glm::mat4 m_combinedMatrix{1.0f};
    glm::mat4 m_inverseCombinedMatrix{1.0f};
};
