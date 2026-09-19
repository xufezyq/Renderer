#pragma once

#include "Camera.h"
#include "Sphere.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

struct Color
{
    float r;
    float g;
    float b;
};

class Renderer
{
public:
    Renderer(unsigned width, unsigned height);
    Color RednerPixel(int x, int y) const;
    int run();

private:
    void renderWorker();

    Camera m_camera;
    Sphere m_sphere;
    unsigned m_viewportWidth;
    unsigned m_viewportHeight;
    std::vector<std::uint32_t> m_displayBuffer;
    std::vector<std::uint32_t> m_renderBuffer;
    std::mutex m_bufferMutex;
    std::vector<std::size_t> m_completedPixels;
    std::atomic<std::size_t> m_nextPixel{0};
    std::atomic<bool> m_stopRendering{false};
};
