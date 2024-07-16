#include "application.hpp"

namespace uka
{
    Uka_application::Uka_application()
    {
        camera.type = Uka_Camera::camera_type::look_at;
        camera.movement_speed = 1.0f;
        camera.rotation_speed = 1.0f;
        camera.position = glm::vec3(0.0f, 0.0f, 1.0f);
        camera.set_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.set_perspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
    }

    Uka_application::~Uka_application()
    {
        if(frame_buffers.deferred)
        {
            delete frame_buffers.deferred;
        }
        if(frame_buffers.shadow)
        {
            delete frame_buffers.shadow;
        }
        vkDestroyPipeline(device, pipelines.deferred_pass, nullptr);
        vkDestroyPipeline(device, pipelines.shadow_pass, nullptr);
        vkDestroyPipeline(device, pipelines.shadow_pass, nullptr);
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(device, desciptor_set_layout, nullptr);

        uniform_buffer_set.composition.destroy();
        uniform_buffer_set.offscreen.destroy();
        uniform_buffer_set.shadows.destroy();

        model_textures.normal.destroy();
        model_textures.color.destroy();
        background_textures.normal.destroy();
        background_textures.color.destroy();
    }

    void Uka_application::init_vulkan()
    {

    }
} // namespace uka

