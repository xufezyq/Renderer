#include "Renderer.h"
#include "RenderTiming.h"

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

Renderer::Renderer(
    unsigned width,
    unsigned height,
    bool enable_ssaa,
    unsigned samples_per_axis)
    : m_sphere(glm::vec3(0.0f, 0.0f, 3.0f), 0.7f),
      m_disk(glm::vec3(1.55f, 0.0f, 3.0f), 0.7f),
      m_triangle(
          glm::vec3(-2.25f, -0.7f, 3.0f),
          glm::vec3(-0.85f, -0.7f, 3.0f),
          glm::vec3(-1.55f, 0.7f, 3.0f)),
      m_enableSsaa(enable_ssaa),
      m_samplesPerAxis(samples_per_axis),
      m_viewportWidth(width),
      m_viewportHeight(height),
      m_displayBuffer(static_cast<std::size_t>(width) * height),
      m_renderBuffer(static_cast<std::size_t>(width) * height)
{
    if (width == 0 || height == 0)
        throw std::invalid_argument("Renderer dimensions must be greater than zero");
    if (samples_per_axis == 0)
        throw std::invalid_argument("SSAA samples per axis must be greater than zero");

    // 左手坐标系中，相机位于原点并朝向 +Z。
    m_camera.Initialize(
        glm::vec3(0.0f),
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
    if (!m_enableSsaa)
    {
        return renderSample(
            static_cast<float>(x) + 0.5f,
            static_cast<float>(y) + 0.5f);
    }

    const float samples_per_axis = static_cast<float>(m_samplesPerAxis);
    const float sample_scale = 1.0f / samples_per_axis;
    const float sample_weight = 1.0f / (samples_per_axis * samples_per_axis);

    Color accumulated_color{};
    for (unsigned sample_y = 0; sample_y < m_samplesPerAxis; ++sample_y)
    {
        for (unsigned sample_x = 0; sample_x < m_samplesPerAxis; ++sample_x)
        {
            const float offset_x = (static_cast<float>(sample_x) + 0.5f) * sample_scale;
            const float offset_y = (static_cast<float>(sample_y) + 0.5f) * sample_scale;
            const Color sample_color = renderSample(
                static_cast<float>(x) + offset_x,
                static_cast<float>(y) + offset_y);

            accumulated_color.r += sample_color.r * sample_weight;
            accumulated_color.g += sample_color.g * sample_weight;
            accumulated_color.b += sample_color.b * sample_weight;
        }
    }

    return accumulated_color;
}

Color Renderer::renderSample(float x, float y) const
{
    // 生成一条从摄像机出发并穿过当前像素的世界空间射线。
    const Ray ray = m_camera.GetRay(x, y);
    Intersection candidate{};
    float closest_t = ray.maxT;
    Color closest_color{};
    bool has_hit = false;

    if (m_sphere.Intersect(ray, candidate))
    {
        // 将法线分量从 [-1, 1] 映射到可显示的 [0, 1] 颜色范围。
        closest_t = candidate.t;
        closest_color = Color{
            candidate.normal.x * 0.5f + 0.5f,
            candidate.normal.y * 0.5f + 0.5f,
            candidate.normal.z * 0.5f + 0.5f};
        has_hit = true;
    }

    if (m_disk.Intersect(ray, candidate) && candidate.t < closest_t)
    {
        closest_t = candidate.t;
        closest_color = Color{1.0f, 1.0f, 0.0f};
        has_hit = true;
    }

    if (m_triangle.Intersect(ray, candidate) && candidate.t < closest_t)
    {
        closest_t = candidate.t;
        closest_color = Color{0.0f, 1.0f, 1.0f};
        has_hit = true;
    }

    if (has_hit)
        return closest_color;

    return Color{
        x / static_cast<float>(m_viewportWidth),
        y / static_cast<float>(m_viewportHeight),
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
    mfb_window* window = mfb_open_ex(
        "FortuneRenderer - Rendering...",
        m_viewportWidth,
        m_viewportHeight,
        MFB_WF_RESIZABLE);
    if (window == nullptr)
        return 0;

    const auto render_start_time = std::chrono::steady_clock::now();
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
    std::size_t displayed_pixel_count = 0;
    bool render_time_displayed = false;

    do
    {
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            completedPixels.swap(m_completedPixels);
        }
        displayed_pixel_count += completedPixels.size();
        for (const std::size_t index : completedPixels)
            m_displayBuffer[index] = m_renderBuffer[index];
        completedPixels.clear();

        state = mfb_update_ex(window, m_displayBuffer.data(), m_viewportWidth, m_viewportHeight);
        if (state != MFB_STATE_OK)
            break;

        if (!render_time_displayed && displayed_pixel_count >= m_displayBuffer.size())
        {
            const auto render_end_time = std::chrono::steady_clock::now();
            const double elapsed_milliseconds =
                std::chrono::duration<double, std::milli>(render_end_time - render_start_time).count();
            const std::string title = FormatRenderTime(elapsed_milliseconds);
            mfb_set_title(window, title.c_str());
            render_time_displayed = true;
        }
    } while (mfb_wait_sync(window));

    m_stopRendering.store(true);
    for (auto& thread : renderThreads)
        thread.join();
    mfb_close(window);
    return 0;
}
