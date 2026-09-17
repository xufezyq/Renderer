#include "WindowLayout.h"

#include <cassert>

int main()
{
    const WorkArea area{0, 0, 2560, 1392};
    const auto windows = ArrangeWindows(area, 120, 90, 16, 39);
    assert(windows[0].x >= area.x);
    assert(windows[0].y >= area.y);
    for (int i = 0; i < 3; ++i)
    {
        assert(windows[i].width > 0 && windows[i].height > 0);
        assert(windows[i].x + windows[i].width <= area.x + area.width);
        assert(windows[i].y + windows[i].height <= area.y + area.height);
        if (i > 0)
        {
            assert(windows[i - 1].x + windows[i - 1].width < windows[i].x);
        }
    }

    const auto small = ArrangeWindows({2560, 0, 900, 500}, 160, 120, 16, 39);
    assert(small[2].x + small[2].width <= 3460);
    assert(small[2].y + small[2].height <= 500);
    return 0;
}
