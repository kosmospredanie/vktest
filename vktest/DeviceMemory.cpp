#include "DeviceMemory.hpp"
#include <stdexcept>

vktest::DeviceMemory::DeviceMemory (const Device &device, VkDeviceSize size, uint32_t memory_type_index)
        : _device {&device} {
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = size;
    alloc_info.memoryTypeIndex = memory_type_index;
    VkResult res = vkAllocateMemory(device.get_native(), &alloc_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to allocate device memory");
}

vktest::DeviceMemory::DeviceMemory (DeviceMemory &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::DeviceMemory::~DeviceMemory () {
    if (_native != nullptr) vkFreeMemory(_device->get_native(), _native, nullptr);
}

VkDeviceMemory vktest::DeviceMemory::get_native () const noexcept {
    return _native;
}

void *vktest::DeviceMemory::map (VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) const noexcept {
    void *data;
    VkResult res = vkMapMemory(_device->get_native(), _native, offset, size, flags, &data);
    return data;
}

void vktest::DeviceMemory::unmap () const noexcept {
    vkUnmapMemory(_device->get_native(), _native);
}
