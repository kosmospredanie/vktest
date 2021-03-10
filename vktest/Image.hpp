#ifndef __VKTEST_IMAGE_HPP__
#define __VKTEST_IMAGE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    class Image {
    public:
        Image (const Device &device, const VkImageCreateInfo &info);
        Image (const Image &) = delete;
        Image (Image &&other) noexcept;
        ~Image ();
        VkImage get_native () const noexcept;
        VkMemoryRequirements get_memory_requirements () const noexcept;
        void bind_memory (const DeviceMemory &memory, VkDeviceSize memory_offset) const noexcept;

    private:
        Image (const Device &device, VkImage native) noexcept;

        VkImage _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_IMAGE_HPP__ */
