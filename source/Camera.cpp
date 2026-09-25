#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

void Camera::Initialize(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, float verticalFov, float nearClip, float farClip, unsigned width, unsigned height)
{
    // 透视投影要求视场角处于有效的开区间内。
    if (verticalFov <= 0.0f || verticalFov >= 180.0f)
    {
        throw std::invalid_argument("Camera vertical FOV must be between 0 and 180 degrees");
    }
    if (nearClip <= 0.0f || farClip <= nearClip || width == 0 || height == 0)
    {
        throw std::invalid_argument("Camera clipping planes or viewport dimensions are invalid");
    }
    if (glm::length(forward) == 0.0f || glm::length(up) == 0.0f)
    {
        throw std::invalid_argument("Camera direction vectors must not be zero");
    }

    if (glm::length(glm::cross(forward, up)) < 0.000001f)
    {
        throw std::invalid_argument("Camera forward and up vectors must not be parallel");
    }

    m_position = position;

    // 左手坐标系中，相机沿 forward 指向观察目标。
    m_viewMatrix = glm::lookAtLH(position, position + glm::normalize(forward), glm::normalize(up));

    // 使用 [0, 1] 的 NDC 深度范围，便于后续进行深度测试。
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    m_projectionMatrix = glm::perspectiveLH_ZO(glm::radians(verticalFov), aspectRatio, nearClip, farClip);

    // 将 NDC 的 X、Y 从 [-1, 1] 映射到屏幕像素坐标。
    // 屏幕 Y 轴向下增长，所以 Y 方向需要翻转；Z 已位于 [0, 1]。
    const float halfWidth = static_cast<float>(width) * 0.5f;
    const float halfHeight = static_cast<float>(height) * 0.5f;

    // GLM 按列接收构造参数，下面依次展开矩阵的四列。
    m_viewportMatrix = glm::mat4(halfWidth, 0.0f, 0.0f, 0.0f, 0.0f, -halfHeight, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, halfWidth, halfHeight, 0.0f, 1.0f);

    // 顶点依次经过观察、投影和视口变换；使用结果前仍需除以 w。
    m_combinedMatrix = m_viewportMatrix * m_projectionMatrix * m_viewMatrix;
    m_inverseCombinedMatrix = glm::inverse(m_combinedMatrix);
}

const glm::mat4& Camera::GetViewMatrix() const
{
    return m_viewMatrix;
}

const glm::mat4& Camera::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

const glm::mat4& Camera::GetViewportMatrix() const
{
    return m_viewportMatrix;
}

const glm::mat4& Camera::GetCombinedMatrix() const
{
    return m_combinedMatrix;
}

const glm::mat4& Camera::GetInverseCombinedMatrix() const
{
    return m_inverseCombinedMatrix;
}

Ray Camera::GetRay(float x, float y) const
{
    Ray ray;
    ray.origin = m_position;

    // 屏幕坐标的 Z 为 0，表示位于近裁剪面。
    const glm::vec4 screenPosition(x, y, 0.0f, 1.0f);
    glm::vec4 worldPosition = m_inverseCombinedMatrix * screenPosition;

    // 齐次坐标除以 w 后，得到近裁剪面上的世界坐标。
    worldPosition /= worldPosition.w;
    ray.direction = glm::normalize(glm::vec3(worldPosition) - m_position);
    return ray;
}
