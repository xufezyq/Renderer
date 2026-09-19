#include "RenderTiming.h"

#include <iomanip>
#include <sstream>

std::string FormatRenderTime(double elapsed_milliseconds)
{
    std::ostringstream title;
    title << "FortuneRenderer - Render time: "
          << std::fixed << std::setprecision(2)
          << elapsed_milliseconds << " ms";
    return title.str();
}
