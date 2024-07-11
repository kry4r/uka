#include <string>
#include <numeric>
#include <array>
#include "uka-vulkan/uka-camera.hpp"
#include "uka-vulkan/uka-buffer.hpp"


namespace uka{
    struct Uka_application
    {
        Uka_application();
        ~Uka_application();
        auto init_vulkan()->void;
        auto setup_window()->void;
        auto prepare()->void;
        auto render()->void;

    private:
        uka::Camera camera;
        int width, height;
    };
}
