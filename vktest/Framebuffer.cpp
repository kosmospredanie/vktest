#include "Framebuffer.hpp"
#include <stdexcept>

vktest::Framebuffer::Framebuffer (const Device &device,
                                  const RenderPass &render_pass,
                                  const VkExtent2D &size,
                                  const std::vector<const ImageView*> &attachments) : _device {&device} {
    std::vector<VkImageView> views {};
    views.reserve( attachments.size() );
    for (const ImageView *attachment : attachments) {
        views.push_back( attachment->get_native() );
    }

    VkFramebufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass.get_native(); // renderPass the framebuffer needs to be compatible
    create_info.attachmentCount = static_cast<uint32_t>( views.size() );
    create_info.pAttachments = views.data();
    create_info.width = size.width;
    create_info.height = size.height;
    create_info.layers = 1; // the number of layers in image arrays

    VkResult res = vkCreateFramebuffer(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create framebuffer");
}

vktest::Framebuffer::Framebuffer (Framebuffer &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Framebuffer::~Framebuffer () {
    if (_native != nullptr) vkDestroyFramebuffer(_device->get_native(), _native, nullptr);
}

VkFramebuffer vktest::Framebuffer::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::Framebuffer::get_device () const noexcept {
    return *_device;
}
