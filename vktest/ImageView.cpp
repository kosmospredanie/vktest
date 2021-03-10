#include "ImageView.hpp"
#include <stdexcept>

vktest::ImageView::ImageView (const Device &device,
                              VkImage image,
                              VkFormat format,
                              VkImageAspectFlags aspect_flags,
                              uint32_t mip_levels) : _device {&device} {
    VkImageViewCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;

    // VK_COMPONENT_SWIZZLE_IDENTITY = 0
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // The subresourceRange field describes what the image's purpose is and
    // which part of the image should be accessed.
    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = mip_levels;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create image view");
}

vktest::ImageView::ImageView (ImageView &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::ImageView::~ImageView () {
    if (_native != nullptr) vkDestroyImageView(_device->get_native(), _native, nullptr);
}

VkImageView vktest::ImageView::get_native () const noexcept {
    return _native;
}
