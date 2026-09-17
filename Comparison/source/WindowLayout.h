#pragma once

#include <array>

struct WorkArea
{
    int x;
    int y;
    int width;
    int height;
};

struct WindowRect
{
    int x;
    int y;
    int width;
    int height;
};

std::array<WindowRect, 3> ArrangeWindows(WorkArea area, unsigned imageWidth,
                                         unsigned imageHeight, int frameWidth, int frameHeight);
