#include "Buffer.hpp"
#include <stdexcept>

vktest::Buffer::Buffer (const Device &device,
                        VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkSharingMode sharing_mode) : _device {&device} {
    VkBufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    // Just like the images in the swap chain, buffers can also be owned by a
    // specific queue family or be shared between multiple at the same time.
    create_info.sharingMode = sharing_mode;

    VkResult res = vkCreateBuffer(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer");
}

vktest::Buffer::Buffer (Buffer &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Buffer::~Buffer () {
    if (_native != nullptr) vkDestroyBuffer(_device->get_native(), _native, nullptr);
}

VkBuffer vktest::Buffer::get_native () const noexcept {
    return _native;
}

VkMemoryRequirements vktest::Buffer::get_memory_requirements () const noexcept {
    VkMemoryRequirements memreq {};
    vkGetBufferMemoryRequirements(_device->get_native(), _native, &memreq);
    return memreq;
}

void vktest::Buffer::bind_memory (const DeviceMemory &memory, VkDeviceSize memory_offset) const noexcept {
    VkResult res = vkBindBufferMemory(_device->get_native(), _native, memory.get_native(), memory_offset);
}
