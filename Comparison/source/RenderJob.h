#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

struct RenderedPixel
{
    std::size_t index;
    std::uint32_t color;
};

class RenderJob
{
public:
    RenderJob(unsigned width, unsigned height, std::chrono::milliseconds delay);
    std::size_t pixelCount() const;
    RenderedPixel renderPixel(std::size_t index) const;

private:
    unsigned m_width;
    unsigned m_height;
    std::chrono::milliseconds m_delay;
};
