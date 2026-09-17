#include "Renderer.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>

#include <MiniFB.h>

namespace
{
    std::uint8_t ToByte(float value)
    {
        const float clamped = std::clamp(value, 0.0f, 1.0f);
        return static_cast<std::uint8_t>(clamped * 255.0f);
    }

    std::uint32_t PackColor(const Color& color)
    {
        return (0xFFu << 24) |
               (static_cast<std::uint32_t>(ToByte(color.b)) << 16) |
               (static_cast<std::uint32_t>(ToByte(color.g)) << 8) |
               static_cast<std::uint32_t>(ToByte(color.r));
    }
}

Renderer::Renderer(unsigned width, unsigned height)
    : m_viewportWidth(width),
      m_viewportHeight(height),
      m_displayBuffer(static_cast<std::size_t>(width) * height),
      m_renderBuffer(static_cast<std::size_t>(width) * height)
{
    if (width == 0 || height == 0)
        throw std::invalid_argument("Renderer dimensions must be greater than zero");

    // 左手坐标系中，相机位于 Z 轴负方向并朝向 +Z。
    m_camera.Initialize(
        glm::vec3(0.0f, 0.0f, -3.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        60.0f,
        0.1f,
        1000.0f,
        width,
        height);
}

Color Renderer::RednerPixel(int x, int y) const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 生成一条从摄像机出发并穿过当前像素的世界空间射线。
    const Ray ray = m_camera.GetRay(x, y);
    (void)ray;

    return Color{
        static_cast<float>(x) / static_cast<float>(m_viewportWidth),
        static_cast<float>(y) / static_cast<float>(m_viewportHeight),
        0.0f};
}

void Renderer::renderWorker()
{
    const std::size_t pixelCount = m_renderBuffer.size();
    while (!m_stopRendering.load())
    {
        const std::size_t index = m_nextPixel.fetch_add(1);
        if (index >= pixelCount)
            return;

        const unsigned x = static_cast<unsigned>(index % m_viewportWidth);
        const unsigned y = static_cast<unsigned>(index / m_viewportWidth);
        const Color color = RednerPixel(static_cast<int>(x), static_cast<int>(y));
        if (m_stopRendering.load())
            return;

        std::lock_guard<std::mutex> lock(m_bufferMutex);
        m_renderBuffer[index] = PackColor(color);
        m_completedPixels.push_back(index);
    }
}

int Renderer::run()
{
    mfb_window* window = mfb_open_ex("my display", m_viewportWidth, m_viewportHeight, MFB_WF_RESIZABLE);
    if (window == nullptr)
        return 0;

    const unsigned availableThreads = std::thread::hardware_concurrency();
    const std::size_t workerCount = std::min<std::size_t>(
        availableThreads == 0 ? 1 : availableThreads, m_renderBuffer.size());
    std::vector<std::thread> renderThreads;
    renderThreads.reserve(workerCount);
    try
    {
        for (std::size_t i = 0; i < workerCount; ++i)
            renderThreads.emplace_back(&Renderer::renderWorker, this);
    }
    catch (...)
    {
        m_stopRendering.store(true);
        for (auto& thread : renderThreads)
            thread.join();
        mfb_close(window);
        throw;
    }

    mfb_update_state state = MFB_STATE_OK;
    std::vector<std::size_t> completedPixels;

    do
    {
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            completedPixels.swap(m_completedPixels);
        }
        for (const std::size_t index : completedPixels)
            m_displayBuffer[index] = m_renderBuffer[index];
        completedPixels.clear();

        state = mfb_update_ex(window, m_displayBuffer.data(), m_viewportWidth, m_viewportHeight);
        if (state != MFB_STATE_OK)
            break;
    } while (mfb_wait_sync(window));

    m_stopRendering.store(true);
    for (auto& thread : renderThreads)
        thread.join();
    mfb_close(window);
    return 0;
}
