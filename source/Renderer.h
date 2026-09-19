#pragma once

#include "Camera.h"
#include "Disk.h"
#include "Primitive.h"
#include "Sphere.h"
#include "Triangle.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <memory>
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
    Renderer(
        unsigned width,
        unsigned height,
        bool enable_ssaa = true,
        unsigned samples_per_axis = 2);
    Color RednerPixel(int x, int y) const;
    int run();

private:
    Color renderSample(float x, float y) const;
    void renderWorker();

    Camera m_camera;
    std::vector<std::unique_ptr<Primitive>> m_primitives;
    bool m_enableSsaa;
    unsigned m_samplesPerAxis;
    unsigned m_viewportWidth;
    unsigned m_viewportHeight;
    std::vector<std::uint32_t> m_displayBuffer;
    std::vector<std::uint32_t> m_renderBuffer;
    std::mutex m_bufferMutex;
    std::vector<std::size_t> m_completedPixels;
    std::atomic<std::size_t> m_nextPixel{0};
    std::atomic<bool> m_stopRendering{false};
};
