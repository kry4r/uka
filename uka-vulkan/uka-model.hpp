#pragma once

#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "uka-device.hpp"

#include "ktx.h"
#include "ktxvulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace uka
{
    namespace gltf
    {
        enum DescriptorBindingFlags
        {
            image_base_color =0x00000001,
            image_normal_map =0x00000002,
        };

        extern VkDescriptorSetLayout descriptor_set_layout_image;
        extern VkDescriptorSetLayout descriptor_set_layout_ubo;
        extern VkMemoryPropertyFlags memory_property_flags;
        extern uint32_t descriptor_binding_flags;

        struct Node;

        struct Texture
        {
            uka::Uka_Device* device = nullptr;
            VkImage image;
            VkImageLayout image_layout;
            VkDeviceMemory device_memory;
            VkImageView image_view;
            uint32_t width, height;
            uint32_t mip_levels;
            uint32_t layer_count;
            VkDescriptorImageInfo descriptor;
            VkSampler sampler;
            uint32_t index;

            auto update_descriptor() -> void;
            auto destroy() -> void;
            auto from_gltf_image(const tinygltf::Image& gltf_image,std::string path, uka::Uka_Device* device, VkQueue copy_queue) -> void;
        };

        struct Material
        {
            uka::Uka_Device* device = nullptr;
            enum AlphaMode
            {
                APLHA_OPAQUE,
                APLHA_MASK,
                APLHA_BLEND
            } alpha_mode;
            AlphaMode alpha_mode_enum = AlphaMode::APLHA_OPAQUE;
            float alpha_cutoff = 1.0f;
            float metallic_factor = 1.0f;
            float roughness_factor = 1.0f;
            glm::vec4 base_color_factor = glm::vec4(1.0f);
            Texture* base_color_texture = nullptr;
            Texture* normal_texture = nullptr;
            Texture* metallic_roughness_texture = nullptr;
            Texture* occlusion_texture = nullptr;
            Texture* emissive_texture = nullptr;
            Texture* specular_glossiness_texture = nullptr;
            Texture* diffuse_texture = nullptr;

            VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
            Material(uka::Uka_Device* device) : device(device) {}
            auto create_descriptor_set(VkDescriptorPool descriptor_pool,VkDescriptorSetLayout descriptor_set_layout,uint32_t descriptor_binding_flags) -> void;
        };

        struct Primitive
        {
            uint32_t first_index;
            uint32_t index_count;
            uint32_t first_vertex;
            uint32_t vertex_count;
            Material material;

            struct Dimensions
            {
                glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
                glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
                glm::vec3 size;
                glm::vec3 center;
                float radius;
            } dimensions;

            auto set_dimensions(const glm::vec3& min, const glm::vec3& max) -> void;
            Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material) : first_index(firstIndex), index_count(indexCount), material(material) {};
        };

        struct Mesh
        {
            std::vector<Primitive*> primitives;
            uka::Uka_Device* device;
            std::string name;

            struct UniformBuffer
            {
                VkBuffer buffer;
                VkDeviceMemory memory;
                VkDescriptorBufferInfo descriptor;
                VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
                void *mapped;
            } uniform_buffer;

            struct UniformBlock
            {
                glm::mat4 matrix;
                glm::mat4 joint_matrix[64];
                float joint_count{0};
            } uniform_block;

            Mesh(uka::Uka_Device* device,glm::mat4 matrix);
            ~Mesh();
        };

        struct Skin
        {
            std::vector<glm::mat4> inverse_bind_matrices;
            std::vector<Node*> joints;
            Node* skeleton = nullptr;
            std::string name;
        };

        struct Node
        {
            std::vector<Node*> children;
            Node* parent;
            Mesh* mesh;
            Skin* skin;
            glm::mat4 matrix;
            glm::vec3 translation{};
            glm::vec3 scale{1.0f};
            glm::quat rotation{};
            std::string name;
            uint32_t index;
            int32_t skin_index = -1;
            auto update() -> void;
            auto local_matrix_from_node() -> glm::mat4;
            auto get_matrix() -> glm::mat4;
            ~Node();
        };

        struct AnimationChannel
        {
            enum PathType
            {
                TRANSLATION,
                ROTATION,
                SCALE
            } path;
            uint32_t sampler_index;
            Node* target_node;
        };

        struct AnimationSampler
        {
            enum InterpolationType
            {
                LINEAR,
                STEP,
                CUBICSPLINE
            } interpolation;
            std::vector<float> inputs;
            std::vector<glm::vec4> outputs;
        };

        struct Animation
        {
            std::string name;
            std::vector<AnimationSampler> samplers;
            std::vector<AnimationChannel> channels;
            float start;
            float end;

        };

        enum VertexComponent
        {
            POSITION = 0x00000000,
            NORMAL = 0x00000001,
            UV = 0x00000002,
            COLOR = 0x00000003,
            TANGENT = 0x00000004,
            JOINTS = 0x00000005,
            WEIGHTS = 0x00000006,
        };

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv;
            glm::vec4 color;
            glm::vec4 joints_0;
            glm::vec4 weights_0;
            glm::vec4 tangent;
            static VkVertexInputBindingDescription vertex_input_binding_description;
            static std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
            static VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info;
            static auto input_binding_description(uint32_t biding) -> VkVertexInputBindingDescription;
            static auto input_attribute_description(uint32_t binding, uint32_t location, VertexComponent component) -> VkVertexInputAttributeDescription;
            static auto input_attribute_descriptions(uint32_t binding,const std::vector<VertexComponent> components) -> std::vector<VkVertexInputAttributeDescription>;
            static auto pipeline_vertex_input_state(const std::vector<VertexComponent> components) -> VkPipelineVertexInputStateCreateInfo*;
        };

        enum LoadFlags
        {
            NONE = 0x00000000,
            PRE_TRANSFORM_VERTICES = 0x00000001,
            PRE_MULTIPLY_VERTEX_COLORS = 0x00000002,
            FLIP_Y = 0x00000004,
            DONT_LOAD_IMAGES = 0x00000008,
        };

        enum VkRenderingFlags
        {
            BIND_IMAGES = 0x00000001,
            RENDER_OPAQUE_NODES = 0x00000002,
            RENDER_ALPHA_MASKED_NODES = 0x00000004,
            RENDER_ALPHA_BLENDED_NODES = 0x00000008,
        };

        struct Model
        {
        private:
            auto get_texture(uint32_t index) -> Texture*;
            Texture empty_texture;
            auto create_empty_texture(VkQueue queue) -> void;
        public:
            uka::Uka_Device* device;
            VkDescriptorPool descriptor_pool;
            struct Vertices
            {
                int count;
                VkBuffer buffer;
                VkDeviceMemory memory;
            } vertices;
            struct Indices
            {
                int count;
                VkBuffer buffer;
                VkDeviceMemory memory;
            } indices;

            std::vector<Node*> nodes;
            std::vector<Node*> linear_nodes;

            std::vector<Skin*> skins;
            std::vector<Texture> textures;
            std::vector<Material> materials;
            std::vector<Animation> animations;

            struct Dimensions
            {
                glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
                glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
                glm::vec3 size;
                glm::vec3 center;
                float radius;
            } dimensions;

            bool metallic_roughness_workflow = true;
            bool buffers_bound = false;
            std::string path;

            Model(){};
            ~Model();
            auto load_node(Node* parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model, std::vector<uint32_t>& index_buffer, std::vector<Vertex>& vertex_buffer, float global_scale) ->void;
            auto load_skins(tinygltf::Model& gltf_model) ->void;
            auto load_image(tinygltf::Model& gltf_model, uka::Uka_Device* device, VkQueue transfer_queue) ->void;
            auto load_materials(tinygltf::Model& gltf_model) ->void;
            auto load_animations(tinygltf::Model& gltf_model) ->void;
            auto load_form_file(std::string filename, uka::Uka_Device* device, VkQueue transfer_queue, uka::gltf::LoadFlags file_loading_flags = LoadFlags::NONE, float scale = 1.0f) ->void;
            auto bind_buffers(VkCommandBuffer commandbuffer) ->void;
            auto draw_node(Node* node, VkCommandBuffer commandbuffer, uint32_t render_flags = 0, VkPipelineLayout pipeline_layout = VK_NULL_HANDLE, uint32_t bind_image_set = 1) ->void;
            auto draw(VkCommandBuffer commandbuffer, uint32_t render_flags = 0, VkPipelineLayout pipeline_layout = VK_NULL_HANDLE, uint32_t bind_image_set = 1) ->void;
            auto get_node_dimensions(Node* node, glm::vec3& min, glm::vec3& max) ->void;
            auto get_scene_dimensions() ->void;
            auto updateAnimation(uint32_t index, float time) ->void;
            auto find_node(Node* parent, uint32_t index) -> Node*;
            auto node_from_index(uint32_t index) -> Node*;
            auto prepare_node_descriptor_set(Node* node,VkDescriptorSetLayout descriptor_set_layout) ->void;
        };
    };
}
