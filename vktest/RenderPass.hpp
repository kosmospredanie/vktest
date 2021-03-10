#ifndef __VKTEST_RENDERPASS_HPP__
#define __VKTEST_RENDERPASS_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include <vector>

namespace vktest {
    class RenderPass {
    public:
        RenderPass (const Device &device,
                VkFormat format, VkFormat depth_format, VkSampleCountFlagBits msaa_samples,
                const std::vector<VkSubpassDependency> &dependencies);
        RenderPass (const RenderPass &) = delete;
        RenderPass (RenderPass &&other) noexcept;
        ~RenderPass ();
        VkRenderPass get_native () const noexcept;
        const Device &get_device () const noexcept;

    private:
        std::vector<VkAttachmentDescription> prepare_attachments (
                VkFormat format, VkFormat depth_format, VkSampleCountFlagBits msaa_samples) const noexcept;
        std::vector<VkAttachmentReference> prepare_color_attachment_refs () const noexcept;
        VkAttachmentReference prepare_depth_attachment_ref () const noexcept;
        std::vector<VkAttachmentReference> prepare_resolve_attachment_refs () const noexcept;
        std::vector<VkSubpassDescription> prepare_subpasses (
                const std::vector<VkAttachmentReference> &color_attachment_refs,
                const VkAttachmentReference *depth_attachment_ref,
                const std::vector<VkAttachmentReference> &resolve_attachment_refs) const noexcept;

        VkRenderPass _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_RENDERPASS_HPP__ */
