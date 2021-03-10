#include "CommandBuffer.hpp"
#include <stdexcept>
#include <algorithm>

vktest::CommandBuffer::CommandBuffer (const CommandPool &pool, VkCommandBufferLevel level)
        : _pool {&pool} {
    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pool.get_native();
    alloc_info.level = level;
    alloc_info.commandBufferCount = 1;
    VkResult res = vkAllocateCommandBuffers(pool.get_device().get_native(), &alloc_info, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to allocate command buffer");
}

vktest::CommandBuffer::CommandBuffer (CommandBuffer &&other) noexcept {
    _native = other._native;
    _pool = other._pool;
    other._native = nullptr;
}

vktest::CommandBuffer::~CommandBuffer () {
    if (_native != nullptr) {
        vkFreeCommandBuffers(_pool->get_device().get_native(), _pool->get_native(), 1, &_native);
        _native = nullptr;
    }
}

VkCommandBuffer vktest::CommandBuffer::get_native () const noexcept {
    return _native;
}

const vktest::CommandPool &vktest::CommandBuffer::get_pool () const noexcept {
    return *_pool;
}

void vktest::CommandBuffer::begin (VkCommandBufferUsageFlags flags,
                                   std::optional<VkCommandBufferInheritanceInfo> inheritance_info) const {
    VkCommandBufferBeginInfo info {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = flags; // Optional
    info.pInheritanceInfo = inheritance_info ? &(*inheritance_info) : VK_NULL_HANDLE; // Optional
    VkResult res = vkBeginCommandBuffer(_native, &info);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to begin recording command buffer");
}

void vktest::CommandBuffer::begin_render_pass (const RenderPass &render_pass,
                                               const Framebuffer &framebuffer,
                                               VkRect2D render_area) const noexcept {
    VkRenderPassBeginInfo info {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = render_pass.get_native();
    info.framebuffer = framebuffer.get_native();
    info.renderArea = std::move(render_area);

    // The clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR.
    // NOTE: The order of clearValues should be identical to the order of your
    // attachments.
    std::vector<VkClearValue> clear_values (2);
    clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    // 1.0 = the far view plane
    clear_values[1].depthStencil = {1.0f, 0};

    info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    info.pClearValues = clear_values.data();

    // The final parameter *contents* controls how the drawing commands within
    // the render pass will be provided. It can have one of two values:
    //  * VK_SUBPASS_CONTENTS_INLINE
    //  * VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    vkCmdBeginRenderPass(_native, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void vktest::CommandBuffer::bind_pipeline (const Pipeline &pipeline, VkPipelineBindPoint bind_point) const noexcept {
    vkCmdBindPipeline(_native, bind_point, pipeline.get_native());
}

void vktest::CommandBuffer::bind_vertex_buffers (uint32_t first_binding,
                                                 uint32_t binding_count,
                                                 const std::vector<VkBuffer> &buffers,
                                                 const std::vector<VkDeviceSize> &offsets) const noexcept {
    vkCmdBindVertexBuffers(_native, first_binding, binding_count, buffers.data(), offsets.data());
}

void vktest::CommandBuffer::bind_index_buffer (const Buffer &buffer, VkDeviceSize offset, VkIndexType index_type) const noexcept {
    vkCmdBindIndexBuffer(_native, buffer.get_native(), offset, index_type);
}

void vktest::CommandBuffer::draw (uint32_t vertex_count,
                                  uint32_t instance_count,
                                  uint32_t first_vertex,
                                  uint32_t first_instance) const noexcept {
    vkCmdDraw(_native, vertex_count, instance_count, first_vertex, first_instance);
}

void vktest::CommandBuffer::draw_indexed (uint32_t index_count,
                                          uint32_t instance_count,
                                          uint32_t first_index,
                                          int32_t vertex_offset,
                                          uint32_t first_instance) const noexcept {
    vkCmdDrawIndexed(_native, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void vktest::CommandBuffer::end_render_pass () const noexcept {
    vkCmdEndRenderPass(_native);
}

void vktest::CommandBuffer::copy_buffer (
        const Buffer &src, const Buffer &dest, const std::vector<VkBufferCopy> &regions) const noexcept {
    vkCmdCopyBuffer(_native, src.get_native(), dest.get_native(), static_cast<uint32_t>(regions.size()), regions.data());
}

void vktest::CommandBuffer::copy_buffer (
        const Buffer &src, const Image &dest, VkImageLayout dest_layout,
        const std::vector<VkBufferImageCopy> &regions) const noexcept {
    vkCmdCopyBufferToImage(_native, src.get_native(), dest.get_native(), dest_layout, static_cast<uint32_t>(regions.size()), regions.data());
}

void vktest::CommandBuffer::pipeline_barrier (
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dest_stage_mask,
    VkDependencyFlags dependency_flags,
    const std::vector<VkMemoryBarrier> &barriers
) const noexcept {
    uint32_t count = static_cast<uint32_t>(barriers.size());
    vkCmdPipelineBarrier(_native, src_stage_mask, dest_stage_mask, dependency_flags, count, barriers.data(), 0, nullptr, 0, nullptr);
}

void vktest::CommandBuffer::pipeline_barrier (
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dest_stage_mask,
    VkDependencyFlags dependency_flags,
    const std::vector<VkBufferMemoryBarrier> &barriers
) const noexcept {
    uint32_t count = static_cast<uint32_t>(barriers.size());
    vkCmdPipelineBarrier(_native, src_stage_mask, dest_stage_mask, dependency_flags, 0, nullptr, count, barriers.data(), 0, nullptr);
}

void vktest::CommandBuffer::pipeline_barrier (
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dest_stage_mask,
    VkDependencyFlags dependency_flags,
    const std::vector<VkImageMemoryBarrier> &barriers
) const noexcept {
    uint32_t count = static_cast<uint32_t>(barriers.size());
    vkCmdPipelineBarrier(_native, src_stage_mask, dest_stage_mask, dependency_flags, 0, nullptr, 0, nullptr, count, barriers.data());
}

void vktest::CommandBuffer::pipeline_barrier (
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dest_stage_mask,
    VkDependencyFlags dependency_flags,
    const std::vector<VkMemoryBarrier> *memory_barriers,
    const std::vector<VkBufferMemoryBarrier> *buffer_barriers,
    const std::vector<VkImageMemoryBarrier> *image_barriers
) const noexcept {
    uint32_t m_count = memory_barriers ? static_cast<uint32_t>(memory_barriers->size()) : 0;
    uint32_t b_count = buffer_barriers ? static_cast<uint32_t>(buffer_barriers->size()) : 0;
    uint32_t i_count = image_barriers ? static_cast<uint32_t>(image_barriers->size()) : 0;
    const VkMemoryBarrier *m_barriers = memory_barriers ? memory_barriers->data() : nullptr;
    const VkBufferMemoryBarrier *b_barriers = buffer_barriers ? buffer_barriers->data() : nullptr;
    const VkImageMemoryBarrier *i_barriers = image_barriers ? image_barriers->data() : nullptr;
    vkCmdPipelineBarrier(_native, src_stage_mask, dest_stage_mask, dependency_flags,
            m_count, m_barriers, b_count, b_barriers, i_count, i_barriers);
}

void vktest::CommandBuffer::blit_image (const Image &src, VkImageLayout src_layout,
                                        const Image &dest, VkImageLayout dest_layout,
                                        const std::vector<VkImageBlit> &regions,
                                        VkFilter filter) const noexcept {
    uint32_t region_count = static_cast<uint32_t>(regions.size());
    vkCmdBlitImage(_native,
                   src.get_native(), src_layout,
                   dest.get_native(), dest_layout,
                   region_count, regions.data(),
                   filter);
}

void vktest::CommandBuffer::end () const {
    VkResult res = vkEndCommandBuffer(_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to record command buffer");
}

vktest::CommandBuffer::CommandBuffer (const CommandPool &pool, VkCommandBuffer native) noexcept
        : _native {native}, _pool {&pool} {
}
