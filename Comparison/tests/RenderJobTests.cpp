#include "RenderJob.h"

#include <cassert>
#include <chrono>

int main()
{
    RenderJob job(4, 2, std::chrono::milliseconds(0));
    assert(job.pixelCount() == 8);
    assert(job.renderPixel(0).color == 0xFF000000u);
    assert(job.renderPixel(7).color == 0xFF007FBFu);

    for (std::size_t index = 0; index < job.pixelCount(); ++index)
    {
        const auto pixel = job.renderPixel(index);
        assert(pixel.index == index);
    }

    return 0;
}
