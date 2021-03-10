#include "Semaphore.hpp"
#include <stdexcept>

vktest::Semaphore::Semaphore (const Device &device) : _device {&device} {
    VkSemaphoreCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult res = vkCreateSemaphore(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create semaphore");
}

vktest::Semaphore::Semaphore (Semaphore &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Semaphore::~Semaphore () {
    if (_native != nullptr) vkDestroySemaphore(_device->get_native(), _native, nullptr);
}

VkSemaphore vktest::Semaphore::get_native () const noexcept {
    return _native;
}
