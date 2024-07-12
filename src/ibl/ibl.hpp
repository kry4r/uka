#include "vulkan/vulkan.h"

namespace uka{
    namespace ibl
    {
        struct skybox
        {
            auto generate_LUT() ->void;
            auto generate_irradiance_map() ->void;
            auto generate_prefiltered_map() ->void;
        };
    }
}