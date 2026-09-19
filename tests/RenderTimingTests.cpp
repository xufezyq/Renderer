#include "RenderTiming.h"

#include <cassert>

int main()
{
    assert(FormatRenderTime(0.0) == "FortuneRenderer - Render time: 0.00 ms");
    assert(FormatRenderTime(123.456) == "FortuneRenderer - Render time: 123.46 ms");
    return 0;
}
