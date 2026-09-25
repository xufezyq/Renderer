#include "Light.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
    bool IsFinite(const glm::vec3& value)
    {
        return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
    }

    void ValidateRadiance(const Color& radiance)
    {
        if (!std::isfinite(radiance.r) || !std::isfinite(radiance.g) || !std::isfinite(radiance.b) || radiance.r < 0.0f || radiance.g < 0.0f || radiance.b < 0.0f)
        {
            throw std::invalid_argument("Light radiance must be finite and non-negative");
        }
    }

    glm::vec3 NormalizeDirection(const glm::vec3& direction)
    {
        const float length = glm::length(direction);
        if (!IsFinite(direction) || !std::isfinite(length) || length == 0.0f)
            throw std::invalid_argument("Light direction must be finite and non-zero");
        return direction / length;
    }

    float InfiniteSourceCoordinate(float point, float direction)
    {
        const float infinity = std::numeric_limits<float>::infinity();
        if (direction > 0.0f)
            return -infinity;
        if (direction < 0.0f)
            return infinity;
        return point;
    }
}

DirectionalLight::DirectionalLight(const glm::vec3& direction, const Color& radiance)
    : m_direction(NormalizeDirection(direction)), m_radiance(radiance)
{
    ValidateRadiance(radiance);
}

const glm::vec3& DirectionalLight::GetDirection() const
{
    return m_direction;
}

Color DirectionalLight::GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const
{
    source_pos = glm::vec3(InfiniteSourceCoordinate(p.x, m_direction.x), InfiniteSourceCoordinate(p.y, m_direction.y), InfiniteSourceCoordinate(p.z, m_direction.z));
    return m_radiance;
}

PointLight::PointLight(const glm::vec3& position, const Color& intensity, const glm::vec3& attenuations)
    : m_position(position),
      m_intensity(intensity),
      m_attenuations(attenuations)
{
    if (!IsFinite(position))
        throw std::invalid_argument("Point light position must be finite");
    ValidateRadiance(intensity);
    if (!IsFinite(attenuations) || attenuations.x < 0.0f || attenuations.y < 0.0f || attenuations.z <= 0.0f)
        throw std::invalid_argument("Point light attenuation coefficients must be finite, with a positive constant term");
}

const glm::vec3& PointLight::GetPosition() const
{
    return m_position;
}

const Color& PointLight::GetIntensity() const
{
    return m_intensity;
}

const glm::vec3& PointLight::GetAttenuations() const
{
    return m_attenuations;
}

Color PointLight::GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const
{
    source_pos = m_position;
    const float distance = glm::length(p - m_position);
    const float distance_attenuation = 1.0f / (m_attenuations.z + m_attenuations.y * distance + m_attenuations.x * distance * distance);
    Color radiance{};
    radiance.r = m_intensity.r * distance_attenuation;
    radiance.g = m_intensity.g * distance_attenuation;
    radiance.b = m_intensity.b * distance_attenuation;
    return radiance;
}

SpotLight::SpotLight(const glm::vec3& direction, const glm::vec3& position, const Color& intensity, float inner_angle_degrees, float outer_angle_degrees, const glm::vec3& attenuations)
    : m_direction(NormalizeDirection(direction)),
      m_position(position),
      m_intensity(intensity),
      m_innerAngleDegrees(inner_angle_degrees),
      m_outerAngleDegrees(outer_angle_degrees),
      m_attenuations(attenuations)
{
    if (!IsFinite(position))
        throw std::invalid_argument("Spot light position must be finite");
    ValidateRadiance(intensity);
    if (!std::isfinite(inner_angle_degrees) || !std::isfinite(outer_angle_degrees) || inner_angle_degrees < 0.0f || outer_angle_degrees <= 0.0f || inner_angle_degrees > outer_angle_degrees || outer_angle_degrees >= 180.0f)
    {
        throw std::invalid_argument("Spot light cone angles must satisfy 0 <= inner <= outer < 180 degrees");
    }
    if (!IsFinite(attenuations) || attenuations.x < 0.0f || attenuations.y < 0.0f || attenuations.z <= 0.0f)
        throw std::invalid_argument("Spot light attenuation coefficients must be finite, with a positive constant term");

    m_cosInnerAngle = std::cos(glm::radians(m_innerAngleDegrees));
    m_cosOuterAngle = std::cos(glm::radians(m_outerAngleDegrees));
    m_inverseCosineRange = m_cosInnerAngle == m_cosOuterAngle ? 0.0f : 1.0f / (m_cosInnerAngle - m_cosOuterAngle);
}

const glm::vec3& SpotLight::GetDirection() const
{
    return m_direction;
}

const glm::vec3& SpotLight::GetPosition() const
{
    return m_position;
}

const Color& SpotLight::GetIntensity() const
{
    return m_intensity;
}

float SpotLight::GetInnerAngleDegrees() const
{
    return m_innerAngleDegrees;
}

float SpotLight::GetOuterAngleDegrees() const
{
    return m_outerAngleDegrees;
}

const glm::vec3& SpotLight::GetAttenuations() const
{
    return m_attenuations;
}

Color SpotLight::GetRadiance(const glm::vec3& p, glm::vec3& source_pos) const
{
    source_pos = m_position;
    const glm::vec3 to_point = p - m_position;
    const float distance = glm::length(to_point);
    if (distance == 0.0f)
        return Color{0.0f, 0.0f, 0.0f};

    // 用光线方向与“光源到采样点”方向的夹角计算角度衰减。
    const float cos_angle = glm::dot(m_direction, to_point / distance);
    const float angle_attenuation = m_cosInnerAngle == m_cosOuterAngle ? (cos_angle >= m_cosOuterAngle ? 1.0f : 0.0f) : std::clamp((cos_angle - m_cosOuterAngle) * m_inverseCosineRange, 0.0f, 1.0f);
    if (angle_attenuation == 0.0f)
        return Color{0.0f, 0.0f, 0.0f};

    // 距离衰减沿用点光源的三项公式，再与角度衰减相乘。
    const float distance_attenuation = 1.0f / (m_attenuations.z + m_attenuations.y * distance + m_attenuations.x * distance * distance);
    Color radiance{};
    radiance.r = m_intensity.r * distance_attenuation * angle_attenuation;
    radiance.g = m_intensity.g * distance_attenuation * angle_attenuation;
    radiance.b = m_intensity.b * distance_attenuation * angle_attenuation;
    return radiance;
}
