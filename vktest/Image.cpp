#include "Image.hpp"
#include <stdexcept>

vktest::Image::Image (const Device &device, const VkImageCreateInfo &info) : _device {&device} {
    VkResult res = vkCreateImage(device.get_native(), &info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create image");
}

vktest::Image::Image (Image &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Image::~Image () {
    if (_native != nullptr) vkDestroyImage(_device->get_native(), _native, nullptr);
}

VkImage vktest::Image::get_native () const noexcept {
    return _native;
}

VkMemoryRequirements vktest::Image::get_memory_requirements () const noexcept {
    VkMemoryRequirements mem_reqs {};
    vkGetImageMemoryRequirements(_device->get_native(), _native, &mem_reqs);
    return mem_reqs;
}

void vktest::Image::bind_memory (const DeviceMemory &memory, VkDeviceSize memory_offset) const noexcept {
    vkBindImageMemory(_device->get_native(), _native, memory.get_native(), memory_offset);
}

vktest::Image::Image (const Device &device, VkImage native) noexcept
        : _native {native}, _device {&device} {
}
