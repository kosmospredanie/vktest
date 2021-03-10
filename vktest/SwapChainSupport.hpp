#ifndef __VKTEST_SWAPCHAINSUPPORT_HPP__
#define __VKTEST_SWAPCHAINSUPPORT_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace vktest {
    struct SwapChainSupport {
        bool is_adequate () const noexcept;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
}

#endif /* __VKTEST_SWAPCHAINSUPPORT_HPP__ */
