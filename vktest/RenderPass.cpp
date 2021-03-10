#include "RenderPass.hpp"
#include <stdexcept>

vktest::RenderPass::RenderPass (const Device &device,
                                VkFormat format,
                                VkFormat depth_format,
                                VkSampleCountFlagBits msaa_samples,
                                const std::vector<VkSubpassDependency> &dependencies) : _device {&device} {
    std::vector<VkAttachmentDescription> attachments = prepare_attachments(format, depth_format, msaa_samples);
    std::vector<VkAttachmentReference> color_attachment_refs = prepare_color_attachment_refs();
    VkAttachmentReference depth_attachment_ref = prepare_depth_attachment_ref();
    std::vector<VkAttachmentReference> resolve_attachment_refs = prepare_resolve_attachment_refs();
    std::vector<VkSubpassDescription> subpasses = prepare_subpasses(
            color_attachment_refs, &depth_attachment_ref, resolve_attachment_refs);

    VkRenderPassCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = static_cast<uint32_t>( attachments.size() );
    create_info.pAttachments = attachments.data();
    create_info.subpassCount = static_cast<uint32_t>( subpasses.size() );
    create_info.pSubpasses = subpasses.data();

    create_info.dependencyCount = static_cast<uint32_t>( dependencies.size() );
    create_info.pDependencies = dependencies.data();

    VkResult res = vkCreateRenderPass(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create render pass");
}

vktest::RenderPass::RenderPass (RenderPass &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::RenderPass::~RenderPass () {
    if (_native != nullptr) vkDestroyRenderPass(_device->get_native(), _native, nullptr);
}

VkRenderPass vktest::RenderPass::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::RenderPass::get_device () const noexcept {
    return *_device;
}

std::vector<VkAttachmentDescription> vktest::RenderPass::prepare_attachments (
        VkFormat format, VkFormat depth_format, VkSampleCountFlagBits msaa_samples) const noexcept {
    VkAttachmentDescription color_attachment {};
    color_attachment.format = format;
    color_attachment.samples = msaa_samples;

    // Determines what to do with the data in the attachment before rendering
    // and after rendering.
    //  * loadOp and storeOp: color and depth data
    //  * stencilLoadOp and stencilStoreOp: stencil data
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // The layout of the pixels in memory can change based on what you're
    // trying to do with an image.
    //  * initialLayout: The layout the image will have before the render pass
    // begins. (VK_IMAGE_LAYOUT_UNDEFINED means that we don't care)
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //  * finalLayout: The layout to automatically transition to when the render
    // pass finishes.
    // Multisampled images cannot be presented directly. We first need to
    // resolve them to a regular image. This requirement does not apply to the
    // depth buffer, since it won't be presented at any point.
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment {};
    depth_attachment.format = depth_format;
    depth_attachment.samples = msaa_samples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // This time we don't care about storing the depth data (storeOp), because
    // it will not be used after drawing has finished.
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Resolve multisampled color image into regular attachment
    VkAttachmentDescription color_attachment_resolve {};
    color_attachment_resolve.format = format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return std::vector { color_attachment, depth_attachment, color_attachment_resolve };
}

std::vector<VkAttachmentReference> vktest::RenderPass::prepare_color_attachment_refs () const noexcept {
    VkAttachmentReference color_attachment_ref {};
    // attachment: Specifies which attachment to reference by its index in the
    // attachment descriptions array.
    color_attachment_ref.attachment = 0;
    // layout: Specifies which layout we would like the attachment to have
    // during a subpass that uses this reference. Vulkan will automatically
    // transition the attachment to this layout when the subpass is started.
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return std::vector { color_attachment_ref };
}

std::vector<VkAttachmentReference> vktest::RenderPass::prepare_resolve_attachment_refs () const noexcept {
    VkAttachmentReference color_attachment_resolve_ref {};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return std::vector { color_attachment_resolve_ref };
}

VkAttachmentReference vktest::RenderPass::prepare_depth_attachment_ref () const noexcept {
    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return depth_attachment_ref;
}

std::vector<VkSubpassDescription> vktest::RenderPass::prepare_subpasses (
            const std::vector<VkAttachmentReference> &color_attachment_refs,
            const VkAttachmentReference *depth_attachment_ref,
            const std::vector<VkAttachmentReference> &resolve_attachment_refs) const noexcept {
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>( color_attachment_refs.size() );
    subpass.pColorAttachments = color_attachment_refs.data();
    subpass.pDepthStencilAttachment = depth_attachment_ref;
    subpass.pResolveAttachments = resolve_attachment_refs.data();
    // The following other types of attachments can be referenced by a subpass:
    //  * pInputAttachments: Attachments that are read from a shader
    //  * pResolveAttachments: Attachments used for multisampling color attachments
    //  * pDepthStencilAttachment: Attachment for depth and stencil data
    //  * pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    return std::vector { subpass };
}
