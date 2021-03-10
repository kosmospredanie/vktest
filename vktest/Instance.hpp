#ifndef __VKTEST_INSTANCE_HPP__
#define __VKTEST_INSTANCE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <cstdint>
#include "PhysicalDevice.hpp"
#include "Surface.hpp"

namespace vktest {
    class PhysicalDevice;
    class Surface;

    class Instance {
    public:
        Instance (const std::string &app_name, uint32_t api_version = VK_API_VERSION_1_0);
        Instance (const Instance &) = delete;
        Instance (Instance &&other) noexcept;
        ~Instance ();
        VkInstance get_native () const noexcept;
        PhysicalDevice &get_physical_device () const noexcept;
        void select_physical_device (const Surface &surface);

    private:
        VkInstance _native;
        std::unique_ptr<PhysicalDevice> _physical_device;
    };
}

#endif /* __VKTEST_INSTANCE_HPP__ */
