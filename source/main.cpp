#include "Renderer.h"

#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    unsigned width = 800;
    unsigned height = 600;

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

        Renderer renderer(width, height);
        return renderer.run();
    }
    catch (const std::exception&)
    {
        return 1;
    }
}
