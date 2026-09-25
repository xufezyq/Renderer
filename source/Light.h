#pragma once

#include "Color.h"

#include <glm/glm.hpp>

class Light
{
public:
    virtual ~Light() = default;
    
    // p 是待照明的世界坐标；每种光源都会把自身位置写入 source_pos。
    virtual Color GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const = 0;
};

class DirectionalLight final : public Light
{
public:
    // direction 表示光线从光源射向场景的方向。
    // GetRadiance 写入无穷远处的光源位置；指向光源的方向可由 -GetDirection() 得到。
    DirectionalLight(const glm::vec3& direction, const Color& radiance);

    const glm::vec3& GetDirection() const;
    Color GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const override;

private:
    glm::vec3 m_direction;
    Color m_radiance;
};

class PointLight final : public Light
{
public:
    // 衰减系数的 x、y、z 分别对应距离平方项、距离项和常数项：1 / (z + y * R + x * R²)。
    PointLight(const glm::vec3& position, const Color& intensity, const glm::vec3& attenuations);

    const glm::vec3& GetPosition() const;
    const Color& GetIntensity() const;
    const glm::vec3& GetAttenuations() const;

    Color GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const override;

private:
    glm::vec3 m_position;
    Color m_intensity;
    glm::vec3 m_attenuations;
};

class SpotLight final : public Light
{
public:
    // direction 指向光线传播方向；内外角是相对该方向测量的半锥角，单位为度。
    // 衰减系数的 x、y、z 分别对应 R²、R 和常数项；锥内强度还要乘以内外角之间的渐变值。
    SpotLight(const glm::vec3& direction, const glm::vec3& position, const Color& intensity, float inner_angle_degrees, float outer_angle_degrees, const glm::vec3& attenuations);

    const glm::vec3& GetDirection() const;
    const glm::vec3& GetPosition() const;
    const Color& GetIntensity() const;
    float GetInnerAngleDegrees() const;
    float GetOuterAngleDegrees() const;
    const glm::vec3& GetAttenuations() const;
    Color GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const override;

private:
    glm::vec3 m_direction;
    glm::vec3 m_position;
    Color m_intensity;

    float m_innerAngleDegrees;
    float m_outerAngleDegrees;
    glm::vec3 m_attenuations;
    // 角度固定，预先保存余弦及渐变区间的倒数，避免每次采样重复计算。
    float m_cosInnerAngle;
    float m_cosOuterAngle;
    float m_inverseCosineRange;
};
