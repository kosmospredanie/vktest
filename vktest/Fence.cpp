#include "Fence.hpp"
#include <stdexcept>

vktest::Fence::Fence (const Device &device, VkFenceCreateFlags flags) : _device {&device} {
    VkFenceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = flags;
    VkResult res = vkCreateFence(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create fence");
}

vktest::Fence::Fence (Fence &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Fence::~Fence () {
    if (_native != nullptr) vkDestroyFence(_device->get_native(), _native, nullptr);
}

VkFence vktest::Fence::get_native () const noexcept {
    return _native;
}

void vktest::Fence::wait (uint64_t timeout) const noexcept {
    vkWaitForFences(_device->get_native(), 1, &_native, VK_TRUE, timeout);
}

void vktest::Fence::reset () const noexcept {
    vkResetFences(_device->get_native(), 1, &_native);
}
