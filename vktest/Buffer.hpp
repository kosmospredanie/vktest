#ifndef __VKTEST_BUFFER_HPP__
#define __VKTEST_BUFFER_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include "DeviceMemory.hpp"

namespace vktest {
    class Buffer {
    public:
        Buffer (const Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharing_mode);
        Buffer (const Buffer &) = delete;
        Buffer (Buffer &&other) noexcept;
        ~Buffer ();
        VkBuffer get_native () const noexcept;
        VkMemoryRequirements get_memory_requirements () const noexcept;
        void bind_memory (const DeviceMemory &memory, VkDeviceSize memory_offset) const noexcept;

    private:
        VkBuffer _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_BUFFER_HPP__ */
