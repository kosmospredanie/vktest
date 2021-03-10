#ifndef __VKTEST_COMMANDBUFFER_HPP__
#define __VKTEST_COMMANDBUFFER_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include "CommandPool.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "DescriptorSet.hpp"
#include "Image.hpp"

namespace vktest {
    class CommandPool;

    class CommandBuffer {
    public:
        CommandBuffer (const CommandPool &pool, VkCommandBufferLevel level);
        CommandBuffer (CommandBuffer &&other) noexcept;
        CommandBuffer (const CommandBuffer &) = delete;
        ~CommandBuffer ();
        VkCommandBuffer get_native () const noexcept;
        const CommandPool &get_pool () const noexcept;
        void begin (VkCommandBufferUsageFlags flags = 0, std::optional<VkCommandBufferInheritanceInfo> inheritance_info = std::nullopt) const;
        void begin_render_pass (const RenderPass &render_pass,
                                const Framebuffer &framebuffer,
                                VkRect2D render_area) const noexcept;
        void bind_pipeline (const Pipeline &pipeline, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const noexcept;
        void bind_vertex_buffers (uint32_t first_binding,
                                  uint32_t binding_count,
                                  const std::vector<VkBuffer> &buffers,
                                  const std::vector<VkDeviceSize> &offsets) const noexcept;
        void bind_index_buffer (const Buffer &buffer, VkDeviceSize offset, VkIndexType index_type) const noexcept;
        // void bind_descriptor_sets ( /* TODO */ ) const noexcept;
        void draw (uint32_t vertex_count,
                   uint32_t instance_count,
                   uint32_t first_vertex,
                   uint32_t first_instance) const noexcept;
        void draw_indexed (uint32_t index_count,
                   uint32_t instance_count,
                   uint32_t first_index,
                   int32_t vertex_offset,
                   uint32_t first_instance) const noexcept;
        void end_render_pass () const noexcept;
        void copy_buffer (const Buffer &src, const Buffer &dest,
                          const std::vector<VkBufferCopy> &regions) const noexcept;
        void copy_buffer (const Buffer &src, const Image &dest, VkImageLayout dest_layout,
                          const std::vector<VkBufferImageCopy> &regions) const noexcept;
        void pipeline_barrier (VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dest_stage_mask,
                               VkDependencyFlags dependency_flags,
                               const std::vector<VkMemoryBarrier> &barriers) const noexcept;
        void pipeline_barrier (VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dest_stage_mask,
                               VkDependencyFlags dependency_flags,
                               const std::vector<VkBufferMemoryBarrier> &barriers) const noexcept;
        void pipeline_barrier (VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dest_stage_mask,
                               VkDependencyFlags dependency_flags,
                               const std::vector<VkImageMemoryBarrier> &barriers) const noexcept;
        /**
         * @param src_stage_mask Specifies in which pipeline stage the
         * operations occur that should happen before the barrier.
         * @param dest_stage_mask Specifies the pipeline stage in which
         * operations will wait on the barrier.
         * @param dependency_flags 0 or VK_DEPENDENCY_BY_REGION_BIT
         */
        void pipeline_barrier (VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dest_stage_mask,
                               VkDependencyFlags dependency_flags,
                               const std::vector<VkMemoryBarrier> *memory_barriers,
                               const std::vector<VkBufferMemoryBarrier> *buffer_barriers,
                               const std::vector<VkImageMemoryBarrier> *image_barriers) const noexcept;
        void blit_image (const Image &src, VkImageLayout src_layout,
                         const Image &dest, VkImageLayout dest_layout,
                         const std::vector<VkImageBlit> &regions,
                         VkFilter filter) const noexcept;
        void end () const;

    private:
        CommandBuffer (const CommandPool &pool, VkCommandBuffer native) noexcept;

        // When a pool is destroyed, all command buffers allocated from the
        // pool are freed.
        VkCommandBuffer _native;
        const CommandPool *_pool;

        friend class CommandPool;
    };
}

#endif /* __VKTEST_COMMANDBUFFER_HPP__ */
