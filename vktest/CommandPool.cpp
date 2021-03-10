#include "CommandPool.hpp"
#include <stdexcept>
#include <algorithm>

vktest::CommandPool::CommandPool (const Device &device,
                                  uint32_t queue_family_index,
                                  VkCommandPoolCreateFlags flags) : _device {&device} {
    VkCommandPoolCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_family_index;
    create_info.flags = flags; // Optional

    VkResult res = vkCreateCommandPool(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create command pool");
}

vktest::CommandPool::CommandPool (CommandPool &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::CommandPool::~CommandPool () {
    if (_native != nullptr) vkDestroyCommandPool(_device->get_native(), _native, nullptr);
}

VkCommandPool vktest::CommandPool::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::CommandPool::get_device () const noexcept {
    return *_device;
}

std::vector<vktest::CommandBuffer> vktest::CommandPool::allocate_buffers (uint32_t count, VkCommandBufferLevel level) const {
    std::vector<VkCommandBuffer> native_bufs ( static_cast<size_t>(count) );
    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = _native;
    alloc_info.level = level;
    alloc_info.commandBufferCount = count;
    VkResult res = vkAllocateCommandBuffers(_device->get_native(), &alloc_info, native_bufs.data());
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to allocate command buffers");

    std::vector<CommandBuffer> bufs {};
    bufs.reserve( static_cast<size_t>(count) );
    for (VkCommandBuffer native_buf : native_bufs) {
        CommandBuffer buf { *this, native_buf };
        bufs.push_back( std::move(buf) );
    }
    return bufs;
}

void vktest::CommandPool::free_buffers (std::vector<CommandBuffer> &buffers) const noexcept {
    std::vector<VkCommandBuffer> native_bufs {};
    native_bufs.reserve(buffers.size());
    for (CommandBuffer &buf : buffers) {
        native_bufs.push_back(buf._native);
        buf._native = nullptr;
    }
    vkFreeCommandBuffers(_device->get_native(), _native, static_cast<uint32_t>(native_bufs.size()), native_bufs.data());
}

void vktest::CommandPool::free_buffers (const std::vector<CommandBuffer*> &buffers) const noexcept {
    std::vector<VkCommandBuffer> native_bufs {};
    for (CommandBuffer *buf : buffers) {
        if (buf->_native != nullptr) {
            native_bufs.push_back(buf->_native);
            buf->_native = nullptr;
        }
    }
    vkFreeCommandBuffers(_device->get_native(), _native, static_cast<uint32_t>(native_bufs.size()), native_bufs.data());
}
