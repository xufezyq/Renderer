#include "RenderJob.h"
#include "WindowLayout.h"

#include <MiniFB.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace
{
    using Clock = std::chrono::steady_clock;

    struct Mode
    {
        Mode(const char* name, unsigned width, unsigned height, unsigned workerCount)
            : name(name), buffer(static_cast<std::size_t>(width) * height, 0xFF000000u),
              workerCount(workerCount)
        {
        }

        const char* name;
        mfb_window* window = nullptr;
        std::vector<std::uint32_t> buffer;
        unsigned workerCount;
        std::atomic<std::size_t> nextPixel{0};
        std::atomic<bool> stop{false};
        std::mutex mutex;
        std::vector<RenderedPixel> completed;
        std::vector<std::thread> workers;
        std::size_t shown = 0;
        bool reported = false;
    };

    void Work(Mode& mode, const RenderJob& job)
    {
        while (!mode.stop.load())
        {
            const auto index = mode.nextPixel.fetch_add(1);
            if (index >= job.pixelCount())
            {
                return;
            }

            const auto pixel = job.renderPixel(index);
            if (mode.stop.load())
            {
                return;
            }

            std::lock_guard<std::mutex> lock(mode.mutex);
            mode.completed.push_back(pixel);
        }
    }

    void Show(Mode& mode, const RenderJob& job, unsigned width, unsigned height,
              Clock::time_point started)
    {
        if (mode.window == nullptr)
        {
            return;
        }

        if (mode.workerCount == 0)
        {
            // Keep the UI responsive while doing real pixel work on the main thread.
            for (unsigned i = 0; i < 16 && mode.nextPixel < job.pixelCount(); ++i)
            {
                const auto index = mode.nextPixel.fetch_add(1);
                const auto pixel = job.renderPixel(index);
                mode.buffer[pixel.index] = pixel.color;
                ++mode.shown;
            }
        }
        else
        {
            std::vector<RenderedPixel> ready;
            {
                std::lock_guard<std::mutex> lock(mode.mutex);
                ready.swap(mode.completed);
            }

            for (const auto& pixel : ready)
            {
                mode.buffer[pixel.index] = pixel.color;
                ++mode.shown;
            }
        }

        if (!mode.reported && mode.shown == job.pixelCount())
        {
            const auto elapsed = std::chrono::duration<double>(Clock::now() - started).count();
            const std::string title = std::string(mode.name) + " - " + std::to_string(elapsed) + " s";
            mfb_set_title(mode.window, title.c_str());
            std::cout << title << '\n';
            mode.reported = true;
        }

        if (mfb_update_ex(mode.window, mode.buffer.data(), width, height) != MFB_STATE_OK)
        {
            mode.window = nullptr;
            mode.stop.store(true);
        }
    }

    unsigned ParseDimension(const char* text)
    {
        const auto value = std::stoull(text);
        if (value == 0 || value > std::numeric_limits<unsigned>::max())
        {
            throw std::out_of_range("Invalid dimension");
        }
        return static_cast<unsigned>(value);
    }

#ifdef _WIN32
    RECT PreferredWorkArea()
    {
        RECT secondary{};
        const auto findSecondary = [](HMONITOR monitor, HDC, LPRECT, LPARAM context) -> BOOL
        {
            MONITORINFO info{sizeof(MONITORINFO)};
            if (GetMonitorInfo(monitor, &info) && !(info.dwFlags & MONITORINFOF_PRIMARY))
            {
                *reinterpret_cast<RECT*>(context) = info.rcWork;
                return FALSE;
            }
            return TRUE;
        };

        EnumDisplayMonitors(nullptr, nullptr, findSecondary,
                            reinterpret_cast<LPARAM>(&secondary));
        if (secondary.right > secondary.left && secondary.bottom > secondary.top)
        {
            return secondary;
        }

        MONITORINFO primary{sizeof(MONITORINFO)};
        if (!GetMonitorInfo(MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY), &primary))
        {
            throw std::runtime_error("Could not determine screen work area");
        }
        return primary.rcWork;
    }
#endif
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 1 && argc != 3)
        {
            std::cerr << "Usage: RenderComparison [width height]\n";
            return 1;
        }

        const unsigned width = argc == 3 ? ParseDimension(argv[1]) : 120;
        const unsigned height = argc == 3 ? ParseDimension(argv[2]) : 90;
        RenderJob job(width, height, std::chrono::milliseconds(1));
        const unsigned logicalThreads = std::max(1u, std::thread::hardware_concurrency());
        Mode mainMode("Main thread", width, height, 0);
        Mode singleMode("Main + 1 render thread", width, height, 1);
        Mode multiMode("Main + multiple render threads", width, height,
                       static_cast<unsigned>(std::min<std::size_t>(logicalThreads, job.pixelCount())));
        const std::array<Mode*, 3> modes{&mainMode, &singleMode, &multiMode};

        unsigned windowIndex = 0;
#ifdef _WIN32
        const RECT work = PreferredWorkArea();
        std::array<WindowRect, 3> windowRects{};
        bool layoutReady = false;
#endif
        for (Mode* mode : modes)
        {
            mode->window = mfb_open_ex(mode->name, width, height, MFB_WF_RESIZABLE);
            if (mode->window == nullptr)
            {
                std::cerr << "Could not open window: " << mode->name << '\n';
                for (Mode* opened : modes)
                {
                    if (opened->window != nullptr)
                    {
                        mfb_close(opened->window);
                    }
                }
                return 1;
            }

#ifdef _WIN32
            // Include the actual non-client frame when sizing the three windows.
            if (HWND handle = FindWindowA(nullptr, mode->name))
            {
                if (!layoutReady)
                {
                    RECT outer{}, client{};
                    GetWindowRect(handle, &outer);
                    GetClientRect(handle, &client);
                    windowRects = ArrangeWindows({work.left, work.top, work.right - work.left,
                                                  work.bottom - work.top}, width, height,
                                                 (outer.right - outer.left) - (client.right - client.left),
                                                 (outer.bottom - outer.top) - (client.bottom - client.top));
                    layoutReady = true;
                }
                const auto& rect = windowRects[windowIndex];
                SetWindowPos(handle, nullptr, rect.x, rect.y, rect.width, rect.height,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
#endif
            ++windowIndex;
        }

        mfb_set_target_fps(60);
        const auto started = Clock::now();
        try
        {
            for (Mode* mode : modes)
            {
                for (unsigned i = 0; i < mode->workerCount; ++i)
                {
                    mode->workers.emplace_back(Work, std::ref(*mode), std::cref(job));
                }
            }

            while (mainMode.window || singleMode.window || multiMode.window)
            {
                for (Mode* mode : modes)
                {
                    Show(*mode, job, width, height, started);
                }

                for (Mode* mode : modes)
                {
                    if (mode->window != nullptr)
                    {
                        if (!mfb_wait_sync(mode->window))
                        {
                            mode->window = nullptr;
                            mode->stop.store(true);
                        }
                        break;
                    }
                }
            }
        }
        catch (...)
        {
            for (Mode* mode : modes)
            {
                mode->stop.store(true);
            }
            for (Mode* mode : modes)
            {
                for (auto& worker : mode->workers)
                {
                    worker.join();
                }
            }
            throw;
        }

        for (Mode* mode : modes)
        {
            mode->stop.store(true);
            for (auto& worker : mode->workers)
            {
                worker.join();
            }
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
