#pragma once

#include "Ray.h"

#include <glm/glm.hpp>

class Camera
{
public:
    // 根据相机姿态、透视参数和渲染尺寸创建各级变换矩阵。
    void Initialize(
        const glm::vec3& position, // 相机在世界坐标系中的位置
        const glm::vec3& forward,  // 相机观察方向
        const glm::vec3& up,       // 相机上方向
        float verticalFov,         // 垂直视场角，单位为度
        float nearClip,            // 近裁剪面距离
        float farClip,             // 远裁剪面距离
        unsigned width,            // 渲染区域宽度
        unsigned height);          // 渲染区域高度

    // 以下接口分别提供观察、投影、视口及三者的联合矩阵。
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;
    const glm::mat4& GetViewportMatrix() const;
    const glm::mat4& GetCombinedMatrix() const;
    const glm::mat4& GetInverseCombinedMatrix() const;

    // 根据屏幕像素坐标生成一条从摄像机位置出发的世界空间射线。
    Ray GetRay(int x, int y) const;

private:
    // 使用左手坐标系，投影后的 NDC 深度范围为 [0, 1]。
    glm::vec3 m_position{0.0f};
    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
    glm::mat4 m_viewportMatrix{1.0f};
    glm::mat4 m_combinedMatrix{1.0f};
    glm::mat4 m_inverseCombinedMatrix{1.0f};
};
