#include <string>
#include <numeric>
#include <array>
#include "../uka-vulkan/uka-camera.hpp"
#include "../uka-vulkan/uka-buffer.hpp"
#include "common.hpp"


namespace uka{
    struct Uka_application
    {
        int32_t debug_display_target = 0;
        bool use_shadows = true;

        float z_near = 0.1f;
        float z_far = 1000.0f;
        float fov = 45.0f;

        float depth_bias_constant = 1.25f;
        float depth_bias_slope = 1.75f;

        utils::ModelTextures model_textures;
        utils::ModelTextures background_textures;
        utils::Models models;
        utils::UniformDataOffscreen uniform_data_offscreen;
        utils::UniformDataShadows uniform_data_shadows;
        utils::UniformDataComposition uniform_data_composition;
        utils::Light lights[LIGHT_COUNT];
        utils::UniformBufferSet uniform_buffer_set;
        utils::PipeLines pipelines;
        VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
        utils::DescriptorSets descriptor_sets;
        VkDescriptorSetLayout desciptor_set_layout{VK_NULL_HANDLE};
        utils::FrameBuffers frame_buffers;
        VkCommandBuffer offscreen_command_buffer{VK_NULL_HANDLE};
        VkSemaphore offscreen_semaphore{VK_NULL_HANDLE};


        Uka_Camera camera;
        VkPhysicalDevice physical_device{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};

        Uka_application();
        ~Uka_application();
        auto init_vulkan()->void;
        auto setup_window()->void;
        auto prepare()->void;
        auto render()->void;
    private:
        uint32_t width,height;
        bool resizing = false;
        auto handle_input()->void;
    };
}
