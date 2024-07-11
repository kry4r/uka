#pragma once
#include <vector>
#include <vulkan/vulkan.h>


namespace uka{
    namespace init{
        inline auto memory_allocate_info() -> VkMemoryAllocateInfo {
            auto memory_allocate_info = VkMemoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            return memory_allocate_info;
        }

        inline auto mapped_memory_range() -> VkMappedMemoryRange {
            auto mapped_memory_range = VkMappedMemoryRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            return mapped_memory_range;
        }

        inline auto command_buffer_allocate_info(VkCommandPool cmd_pool, VkCommandBufferLevel level, uint32_t count) -> VkCommandBufferAllocateInfo {
            auto command_buffer_allocate_info = VkCommandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            command_buffer_allocate_info.commandPool = cmd_pool;
            command_buffer_allocate_info.level = level;
            command_buffer_allocate_info.commandBufferCount = count;
            return command_buffer_allocate_info;
        }

        inline auto command_pool_create_info() -> VkCommandPoolCreateInfo {
            auto command_pool_create_info = VkCommandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            return command_pool_create_info;
        }

        inline auto command_buffer_begin_info() -> VkCommandBufferBeginInfo {
            auto command_buffer_begin_info = VkCommandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            return command_buffer_begin_info;
        }

        inline auto command_buffer_inheritance_info() -> VkCommandBufferInheritanceInfo {
            auto command_buffer_inheritance_info = VkCommandBufferInheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
            return command_buffer_inheritance_info;
        }

        inline auto render_pass_begin_info() -> VkRenderPassBeginInfo {
            auto render_pass_begin_info = VkRenderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            return render_pass_begin_info;
        }

        inline auto render_pass_create_info() -> VkRenderPassCreateInfo {
            auto render_pass_create_info = VkRenderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
            return render_pass_create_info;
        }

        inline auto image_memory_barrier() -> VkImageMemoryBarrier {
            auto image_memory_barrier = VkImageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            return image_memory_barrier;
        }

        inline auto buffer_memory_barrier() -> VkBufferMemoryBarrier {
            auto buffer_memory_barrier = VkBufferMemoryBarrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            return buffer_memory_barrier;
        }

        inline auto memory_barrier() -> VkMemoryBarrier {
            auto memory_barrier = VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            return memory_barrier;
        }

        inline auto image_create_info() -> VkImageCreateInfo {
            auto image_create_info = VkImageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            return image_create_info;
        }

        inline auto sampler_create_info() -> VkSamplerCreateInfo {
            auto sampler_create_info = VkSamplerCreateInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
            return sampler_create_info;
        }

        inline auto image_view_create_info() -> VkImageViewCreateInfo {
            auto image_view_create_info = VkImageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            return image_view_create_info;
        }

        inline auto framebuffer_create_info() -> VkFramebufferCreateInfo {
            auto framebuffer_create_info = VkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            return framebuffer_create_info;
        }

        inline auto semaphore_create_info() -> VkSemaphoreCreateInfo {
            auto semaphore_create_info = VkSemaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            return semaphore_create_info;
        }

        inline auto fence_create_info() -> VkFenceCreateInfo {
            auto fence_create_info = VkFenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            return fence_create_info;
        }

        inline auto event_create_info() -> VkEventCreateInfo {
            auto event_create_info = VkEventCreateInfo{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
            return event_create_info;
        }

        inline auto submit_info() -> VkSubmitInfo {
            auto submit_info = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            return submit_info;
        }

        inline auto viewport(float width,float height,float min_depth,float max_depth) -> VkViewport {
            auto viewport = VkViewport{};
            viewport.width = width;
            viewport.height = height;
            viewport.minDepth = min_depth;
            viewport.maxDepth = max_depth;
            return viewport;
        }

        inline auto rect2d(VkOffset2D offset,VkExtent2D extent) -> VkRect2D {
            auto rect2d = VkRect2D{};
            rect2d.offset = offset;
            rect2d.extent = extent;
            return rect2d;
        }

        inline auto buffer_create_info() -> VkBufferCreateInfo {
            auto buffer_create_info = VkBufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            return buffer_create_info;
        }

        inline auto buffer_create_info(VkBufferUsageFlags flag,VkDeviceSize size) -> VkBufferCreateInfo {
            auto buffer_create_info = VkBufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            buffer_create_info.usage = flag;
            buffer_create_info.size = size;
            return buffer_create_info;
        }

        inline auto descriptor_pool_create_info(uint32_t pool_size_count,VkDescriptorPoolSize* psize,uint32_t max_sets) -> VkDescriptorPoolCreateInfo {
            auto descriptor_pool_create_info = VkDescriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            descriptor_pool_create_info.poolSizeCount = pool_size_count;
            descriptor_pool_create_info.pPoolSizes = psize;
            descriptor_pool_create_info.maxSets = max_sets;
            return descriptor_pool_create_info;
        }

        inline auto descriptor_pool_create_info(const std::vector<VkDescriptorPoolSize>& pool_sizes,uint32_t max_sets) -> VkDescriptorPoolCreateInfo {
            auto descriptor_pool_create_info = VkDescriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            descriptor_pool_create_info.poolSizeCount = pool_sizes.size();
            descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
            descriptor_pool_create_info.maxSets = max_sets;
            return descriptor_pool_create_info;
        }

        inline auto descriptor_pool_size(VkDescriptorType type,uint32_t count) -> VkDescriptorPoolSize {
            auto descriptor_pool_size = VkDescriptorPoolSize{};
            descriptor_pool_size.type = type;
            descriptor_pool_size.descriptorCount = count;
            return descriptor_pool_size;
        }

        inline auto descriptor_set_layout_binding(VkDescriptorType type,VkShaderStageFlags stage,uint32_t binding,uint32_t count = 1) -> VkDescriptorSetLayoutBinding {
            auto descriptor_set_layout_binding = VkDescriptorSetLayoutBinding{};
            descriptor_set_layout_binding.descriptorType = type;
            descriptor_set_layout_binding.stageFlags = stage;
            descriptor_set_layout_binding.binding = binding;
            descriptor_set_layout_binding.descriptorCount = count;
            return descriptor_set_layout_binding;
        }

        inline auto descriptor_set_layout_create_info(uint32_t binding_count,const VkDescriptorSetLayoutBinding* bindings) -> VkDescriptorSetLayoutCreateInfo {
            auto descriptor_set_layout_create_info = VkDescriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            descriptor_set_layout_create_info.bindingCount = binding_count;
            descriptor_set_layout_create_info.pBindings = bindings;
            return descriptor_set_layout_create_info;
        }

        inline auto descriptor_set_layout_create_info(const std::vector<VkDescriptorSetLayoutBinding>& bindings) -> VkDescriptorSetLayoutCreateInfo {
            auto descriptor_set_layout_create_info = VkDescriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            descriptor_set_layout_create_info.bindingCount = bindings.size();
            descriptor_set_layout_create_info.pBindings = bindings.data();
            return descriptor_set_layout_create_info;
        }

        inline auto pipeline_layout_create_info(uint32_t set_layout_count,const VkDescriptorSetLayout* set_layouts) -> VkPipelineLayoutCreateInfo {
            auto pipeline_layout_create_info = VkPipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
            pipeline_layout_create_info.setLayoutCount = set_layout_count;
            pipeline_layout_create_info.pSetLayouts = set_layouts;
            return pipeline_layout_create_info;
        }

        inline auto pipeline_layout_create_info(uint32_t set_layout_count) -> VkPipelineLayoutCreateInfo {
            auto pipeline_layout_create_info = VkPipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
            pipeline_layout_create_info.setLayoutCount = set_layout_count;
            return pipeline_layout_create_info;
        }

        inline auto descriptor_set_allocate_info(VkDescriptorPool pool,uint32_t count,const VkDescriptorSetLayout* layouts) -> VkDescriptorSetAllocateInfo {
            auto descriptor_set_allocate_info = VkDescriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            descriptor_set_allocate_info.descriptorPool = pool;
            descriptor_set_allocate_info.descriptorSetCount = count;
            descriptor_set_allocate_info.pSetLayouts = layouts;
            return descriptor_set_allocate_info;
        }

        inline auto descriptor_img_info(VkSampler sampler,VkImageView view,VkImageLayout layout) -> VkDescriptorImageInfo {
            auto descriptor_img_info = VkDescriptorImageInfo{};
            descriptor_img_info.sampler = sampler;
            descriptor_img_info.imageView = view;
            descriptor_img_info.imageLayout = layout;
            return descriptor_img_info;
        }

        inline auto write_descriptor_set(VkDescriptorSet set,uint32_t binding,uint32_t array_element,VkDescriptorType type,const VkDescriptorImageInfo* img_info) -> VkWriteDescriptorSet {
            auto write_descriptor_set = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write_descriptor_set.dstSet = set;
            write_descriptor_set.dstBinding = binding;
            write_descriptor_set.dstArrayElement = array_element;
            write_descriptor_set.descriptorType = type;
            write_descriptor_set.pImageInfo = img_info;
            return write_descriptor_set;
        }

        inline auto write_descriptor_set(VkDescriptorSet set,uint32_t binding,uint32_t array_element,VkDescriptorType type,const VkDescriptorBufferInfo* buffer_info) -> VkWriteDescriptorSet {
            auto write_descriptor_set = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write_descriptor_set.dstSet = set;
            write_descriptor_set.dstBinding = binding;
            write_descriptor_set.dstArrayElement = array_element;
            write_descriptor_set.descriptorType = type;
            write_descriptor_set.pBufferInfo = buffer_info;
            return write_descriptor_set;
        }

        inline auto vertex_input_binding(uint32_t binding,uint32_t stride,VkVertexInputRate rate) -> VkVertexInputBindingDescription {
            auto vertex_input_binding = VkVertexInputBindingDescription{};
            vertex_input_binding.binding = binding;
            vertex_input_binding.stride = stride;
            vertex_input_binding.inputRate = rate;
            return vertex_input_binding;
        }

        inline auto vertex_input_attribute(uint32_t binding,uint32_t location,VkFormat format,uint32_t offset) -> VkVertexInputAttributeDescription {
            auto vertex_input_attribute = VkVertexInputAttributeDescription{};
            vertex_input_attribute.binding = binding;
            vertex_input_attribute.location = location;
            vertex_input_attribute.format = format;
            vertex_input_attribute.offset = offset;
            return vertex_input_attribute;
        }

        inline auto pipeline_vertex_input_state_create_info() -> VkPipelineVertexInputStateCreateInfo {
            auto pipeline_vertex_input_state_create_info = VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            return pipeline_vertex_input_state_create_info;
        }

        inline auto pipeline_vertex_input_state_create_info(uint32_t binding_count,const VkVertexInputBindingDescription* bindings,uint32_t attribute_count,const VkVertexInputAttributeDescription* attributes) -> VkPipelineVertexInputStateCreateInfo {
            auto pipeline_vertex_input_state_create_info = VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = binding_count;
            pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = bindings;
            pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = attribute_count;
            pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = attributes;
            return pipeline_vertex_input_state_create_info;
        }

        inline auto pipeline_vertex_input_state_create_info(const std::vector<VkVertexInputBindingDescription>& bindings,const std::vector<VkVertexInputAttributeDescription>& attributes) -> VkPipelineVertexInputStateCreateInfo {
            auto pipeline_vertex_input_state_create_info = VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = bindings.size();
            pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = bindings.data();
            pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = attributes.size();
            pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = attributes.data();
            return pipeline_vertex_input_state_create_info;
        }

        inline auto pipeline_input_assembly_state_create_info(VkPrimitiveTopology topology,VkPipelineInputAssemblyStateCreateFlags flags,VkBool32 primitiveRestartEnable) -> VkPipelineInputAssemblyStateCreateInfo {
            auto pipeline_input_assembly_state_create_info = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            pipeline_input_assembly_state_create_info.topology = topology;
			pipeline_input_assembly_state_create_info.flags = flags;
			pipeline_input_assembly_state_create_info.primitiveRestartEnable = primitiveRestartEnable;
            return pipeline_input_assembly_state_create_info;
        }

        inline auto pipeline_rasterization_state_create_info(VkPolygonMode polygon_mode,VkCullModeFlags cull_mode,VkFrontFace front_face,VkPipelineRasterizationStateCreateFlags flags) -> VkPipelineRasterizationStateCreateInfo {
            auto pipeline_rasterization_state_create_info = VkPipelineRasterizationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            pipeline_rasterization_state_create_info.polygonMode = polygon_mode;
            pipeline_rasterization_state_create_info.cullMode = cull_mode;
            pipeline_rasterization_state_create_info.frontFace = front_face;
            pipeline_rasterization_state_create_info.flags = flags;
            return pipeline_rasterization_state_create_info;
        }

        inline auto pipeline_color_blend_attachment_state(VkBool32 blend_enable,VkColorComponentFlags color_write_mask) -> VkPipelineColorBlendAttachmentState {
            auto pipeline_color_blend_attachment_state = VkPipelineColorBlendAttachmentState{};
            pipeline_color_blend_attachment_state.blendEnable = blend_enable;
            pipeline_color_blend_attachment_state.colorWriteMask = color_write_mask;
            return pipeline_color_blend_attachment_state;
        }

        inline auto pipeline_depth_stencil_state_create_info(VkBool32 depth_test_enable,VkBool32 depth_write_enable,VkCompareOp depth_compare_op) -> VkPipelineDepthStencilStateCreateInfo {
            auto pipeline_depth_stencil_state_create_info = VkPipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            pipeline_depth_stencil_state_create_info.depthTestEnable = depth_test_enable;
            pipeline_depth_stencil_state_create_info.depthWriteEnable = depth_write_enable;
            pipeline_depth_stencil_state_create_info.depthCompareOp = depth_compare_op;
            return pipeline_depth_stencil_state_create_info;
        }

        inline auto pipeline_viewport_state_create_info(uint32_t viewport_count,uint32_t scissor_count,VkPipelineViewportStateCreateFlags flags = 0) -> VkPipelineViewportStateCreateInfo {
            auto pipeline_viewport_state_create_info = VkPipelineViewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            pipeline_viewport_state_create_info.viewportCount = viewport_count;
            pipeline_viewport_state_create_info.scissorCount = scissor_count;
            pipeline_viewport_state_create_info.flags = flags;
            return pipeline_viewport_state_create_info;
        }

        inline auto pipeline_multisample_state_create_info(VkSampleCountFlagBits rasterization_samples,VkPipelineMultisampleStateCreateFlags flags = 0) -> VkPipelineMultisampleStateCreateInfo {
            auto pipeline_multisample_state_create_info = VkPipelineMultisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
            pipeline_multisample_state_create_info.rasterizationSamples = rasterization_samples;
            pipeline_multisample_state_create_info.flags = flags;
            return pipeline_multisample_state_create_info;
        }

        inline auto pipeline_dynamic_state_create_info(uint32_t dynamic_state_count,const VkDynamicState* dynamic_states,VkPipelineDynamicStateCreateFlags flags = 0) -> VkPipelineDynamicStateCreateInfo {
            auto pipeline_dynamic_state_create_info = VkPipelineDynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_state_count;
            pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states;
            pipeline_dynamic_state_create_info.flags = flags;
            return pipeline_dynamic_state_create_info;
        }

        inline auto pipeline_dynamic_state_create_info(const std::vector<VkDynamicState>& dynamic_states,VkPipelineDynamicStateCreateFlags flags = 0) -> VkPipelineDynamicStateCreateInfo {
            auto pipeline_dynamic_state_create_info = VkPipelineDynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
            pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();
            pipeline_dynamic_state_create_info.flags = flags;
            return pipeline_dynamic_state_create_info;
        }

        inline auto pipeline_tessellation_state_create_info(uint32_t patch_control_points) -> VkPipelineTessellationStateCreateInfo {
            auto pipeline_tessellation_state_create_info = VkPipelineTessellationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
            pipeline_tessellation_state_create_info.patchControlPoints = patch_control_points;
            return pipeline_tessellation_state_create_info;
        }

        inline auto pipeline_create_info(VkPipelineLayout layout,VkRenderPass render_pass,uint32_t subpass,VkPipelineCreateFlags flags = 0) -> VkGraphicsPipelineCreateInfo {
            auto pipeline_create_info = VkGraphicsPipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
            pipeline_create_info.layout = layout;
            pipeline_create_info.renderPass = render_pass;
            pipeline_create_info.subpass = subpass;
            pipeline_create_info.flags = flags;
            return pipeline_create_info;
        }

        inline auto pipeline_create_info() -> VkGraphicsPipelineCreateInfo {
            auto pipeline_create_info = VkGraphicsPipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
            return pipeline_create_info;
        }

        inline auto compute_pipeline_create_info(VkPipelineLayout layout,VkPipelineCreateFlags flags = 0) -> VkComputePipelineCreateInfo {
            auto compute_pipeline_create_info = VkComputePipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
            compute_pipeline_create_info.layout = layout;
            compute_pipeline_create_info.flags = flags;
            return compute_pipeline_create_info;
        }

        inline auto push_constant_range(VkShaderStageFlags stage,uint32_t offset,uint32_t size) -> VkPushConstantRange {
            auto push_constant_range = VkPushConstantRange{};
            push_constant_range.stageFlags = stage;
            push_constant_range.offset = offset;
            push_constant_range.size = size;
            return push_constant_range;
        }

        inline auto bind_sparse_info() -> VkBindSparseInfo {
            auto bind_sparse_info = VkBindSparseInfo{VK_STRUCTURE_TYPE_BIND_SPARSE_INFO};
            return bind_sparse_info;
        }

        inline auto specialization_map_entry(uint32_t constant_id,uint32_t offset,size_t size){
            auto specialization_map_entry = VkSpecializationMapEntry{};
            specialization_map_entry.constantID = constant_id;
            specialization_map_entry.offset = offset;
            specialization_map_entry.size = size;
            return specialization_map_entry;
        }

        inline auto specialization_info(uint32_t map_entry_count,const VkSpecializationMapEntry* map_entries,size_t data_size,const void* data){
            auto specialization_info = VkSpecializationInfo{};
            specialization_info.mapEntryCount = map_entry_count;
            specialization_info.pMapEntries = map_entries;
            specialization_info.dataSize = data_size;
            specialization_info.pData = data;
            return specialization_info;
        }

        inline auto specialization_info(const std::vector<VkSpecializationMapEntry>& map_entries,size_t data_size,const void* data){
            auto specialization_info = VkSpecializationInfo{};
            specialization_info.mapEntryCount = map_entries.size();
            specialization_info.pMapEntries = map_entries.data();
            specialization_info.dataSize = data_size;
            specialization_info.pData = data;
            return specialization_info;
        }
    } // namespace init
} // namespace uka
