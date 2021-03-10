#ifndef __VKTEST_DEVICEMEMORY_HPP__
#define __VKTEST_DEVICEMEMORY_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    class Device;

    class DeviceMemory {
    public:
        DeviceMemory (const Device &device, VkDeviceSize size, uint32_t memory_type_index);
        DeviceMemory (const DeviceMemory &) = delete;
        DeviceMemory (DeviceMemory &&other) noexcept;
        ~DeviceMemory ();
        VkDeviceMemory get_native () const noexcept;
        /**
         * @return data
         */
        void *map (VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags = 0) const noexcept;
        void unmap () const noexcept;

    private:
        VkDeviceMemory _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_DEVICEMEMORY_HPP__ */
