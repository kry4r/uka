#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <chrono>
#include <map>
#include <unordered_map>
#include "algorithm"
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "../uka-vulkan/uka-device.hpp"
#include "../uka-vulkan/uka-texture.hpp"
#include "../uka-vulkan/uka-model.hpp"
#include "../uka-vulkan/uka-framebuffer.hpp"

#define LIGHT_COUNT 3

namespace uka{
    namespace utils
    {

        struct ModelTextures
        {
            Uka_Texture2D color;
            Uka_Texture2D normal;
        };
        struct Textures
        {
            Uka_Texture_Cube enviroment_cube;
            Uka_Texture_Cube irradiance_cube;
            Uka_Texture_Cube prefiltered_cube;
            Uka_Texture2D brdf_lut;
            std::vector<ModelTextures> model_textures;
        };

        struct Models
        {
            gltf::Model model;
            gltf::Model skybox;
        };


        struct UniformDataOffscreen
        {
            glm::mat4 projection{ 1.0f };
            glm::mat4 model{ 1.0f };
            glm::mat4 view{ 1.0f };
            //glm::vec3 camPos{ 0.0f };
            glm::vec4 instancePOS[3];
            int layer{0};
        };

        struct UniformDataShadows
        {
            glm::mat4 MVP[LIGHT_COUNT];
            glm::vec4 instancePOS[3];
        };

        struct Light {
            glm::vec4 position;
            glm::vec4 target;
            glm::vec4 color;
            glm::mat4 view_matrix;
        };

        struct UniformDataComposition
        {
            glm::vec4 viewPos;
            Light lights[LIGHT_COUNT];
            uint32_t use_shadows = 1;
            int32_t debug_display_target = 0;
        };

        struct UniformBufferSet
        {
            Uka_Buffer offscreen;
            Uka_Buffer composition;
            Uka_Buffer shadows;
        };

        struct PipeLines
        {
            VkPipeline offscreen_pass;
            VkPipeline deferred_pass;
            VkPipeline shadow_pass;
        };

        struct DescriptorSets
        {
            VkDescriptorSet model{VK_NULL_HANDLE};
            VkDescriptorSet background{VK_NULL_HANDLE};
            VkDescriptorSet shadow{VK_NULL_HANDLE};
            VkDescriptorSet composition{VK_NULL_HANDLE};
        };

        struct FrameBuffers
        {
            Uka_Framebuffer* deferred;
            Uka_Framebuffer* shadow;
        };




    }

}