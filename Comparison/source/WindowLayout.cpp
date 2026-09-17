#include "WindowLayout.h"

#include <algorithm>
#include <stdexcept>

std::array<WindowRect, 3> ArrangeWindows(WorkArea area, unsigned imageWidth,
                                         unsigned imageHeight, int frameWidth, int frameHeight)
{
    if (area.width <= 0 || area.height <= 0 || imageWidth == 0 || imageHeight == 0)
    {
        throw std::invalid_argument("Invalid screen area or image size");
    }

    constexpr int margin = 12;
    constexpr int gap = 12;
    const int slotWidth = (area.width - 2 * margin - 2 * gap) / 3;
    const int availableHeight = area.height - 2 * margin;
    if (slotWidth <= frameWidth || availableHeight <= frameHeight)
    {
        throw std::invalid_argument("Screen work area is too small for three windows");
    }

    const int clientWidth = std::min(slotWidth - frameWidth,
        static_cast<int>(static_cast<double>(availableHeight - frameHeight) * imageWidth / imageHeight));
    const int width = clientWidth + frameWidth;
    const int height = static_cast<int>(static_cast<double>(clientWidth) * imageHeight / imageWidth) + frameHeight;
    std::array<WindowRect, 3> result{};
    for (int i = 0; i < 3; ++i)
    {
        result[i] = {area.x + margin + i * (slotWidth + gap) + (slotWidth - width) / 2,
                     area.y + (area.height - height) / 2, width, height};
    }
    return result;
}
