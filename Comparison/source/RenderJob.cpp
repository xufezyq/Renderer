#include "RenderJob.h"

#include <stdexcept>
#include <thread>

RenderJob::RenderJob(unsigned width, unsigned height, std::chrono::milliseconds delay)
    : m_width(width), m_height(height), m_delay(delay)
{
    if (width == 0 || height == 0 || delay.count() < 0)
    {
        throw std::invalid_argument("Invalid render dimensions or delay");
    }
}

std::size_t RenderJob::pixelCount() const
{
    return static_cast<std::size_t>(m_width) * m_height;
}

RenderedPixel RenderJob::renderPixel(std::size_t index) const
{
    if (index >= pixelCount())
    {
        throw std::out_of_range("Pixel index out of range");
    }

    std::this_thread::sleep_for(m_delay);
    const auto x = static_cast<std::uint32_t>(index % m_width);
    const auto y = static_cast<std::uint32_t>(index / m_width);
    const auto red = x * 255u / m_width;
    const auto green = y * 255u / m_height;
    return {index, 0xFF000000u | (green << 8) | red};
}
