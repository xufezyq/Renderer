#include "Renderer.h"

#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    unsigned width = 800;
    unsigned height = 450;
    constexpr bool enable_ssaa = true;
    constexpr unsigned ssaa_samples_per_axis = 16;

    try
    {
        if (argc == 3)
        {
            width = static_cast<unsigned>(std::stoul(argv[1]));
            height = static_cast<unsigned>(std::stoul(argv[2]));
        }
        else if (argc != 1)
        {
            return 1;
        }

        Renderer renderer(width, height, enable_ssaa, ssaa_samples_per_axis);
        return renderer.run();
    }
    catch (const std::exception&)
    {
        return 1;
    }
}
