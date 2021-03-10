#ifndef __VKTEST_PHYSICALDEVICE_HPP__
#define __VKTEST_PHYSICALDEVICE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include "QueueFamilyIndices.hpp"
#include "Instance.hpp"
#include "SwapChainSupport.hpp"
#include "Surface.hpp"

namespace vktest {
    class Instance;
    class Surface;

    class PhysicalDevice {
    public:
        PhysicalDevice (const PhysicalDevice &) = delete;
        VkPhysicalDevice get_native () const noexcept;
        const QueueFamilyIndices &get_queue_families () const noexcept;
        SwapChainSupport query_swap_chain_support (const Surface &surface) const noexcept;
        VkPhysicalDeviceMemoryProperties get_memory_properties () const noexcept;
        VkPhysicalDeviceProperties get_properties () const noexcept;
        VkFormatProperties get_format_properties (VkFormat format) const noexcept;

    private:
        static std::unique_ptr<PhysicalDevice> select (const Instance &instance, const Surface &surface);
        PhysicalDevice (VkPhysicalDevice native, const Surface &surface) noexcept;

        /**
         * NOTE: VkPhysicalDevice objects *cannot* be explicitly destroyed.
         * Instead, they are implicitly destroyed when the VkInstance object
         * they are retrieved from is destroyed.
         */
        VkPhysicalDevice _native;
        QueueFamilyIndices _queue_families;

        friend class Instance;
    };
}

#endif /* __VKTEST_PHYSICALDEVICE_HPP__ */
