#include "Renderer.h"

#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    unsigned width = 800;
    unsigned height = 450;
    std::string scene_file_path = "scene.xml";
    constexpr bool enable_ssaa = true;
    constexpr unsigned ssaa_samples_per_axis = 2;

    try
    {
        if (argc == 2)
        {
            scene_file_path = argv[1];
        }
        else if (argc == 3 || argc == 4)
        {
            width = static_cast<unsigned>(std::stoul(argv[1]));
            height = static_cast<unsigned>(std::stoul(argv[2]));
            if (argc == 4)
                scene_file_path = argv[3];
        }
        else if (argc != 1)
        {
            return 1;
        }

        Renderer renderer(
            width,
            height,
            enable_ssaa,
            ssaa_samples_per_axis,
            scene_file_path);
        return renderer.run();
    }
    catch (const std::exception&)
    {
        return 1;
    }
}
