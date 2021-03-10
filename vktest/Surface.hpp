#ifndef __VKTEST_SURFACE_HPP__
#define __VKTEST_SURFACE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "Window.hpp"

namespace vktest {
    class Instance;

    class Surface {
    public:
        Surface (const Instance &instance, const Window &window);
        Surface (const Surface &) = delete;
        Surface (Surface &&other) noexcept;
        ~Surface ();
        VkSurfaceKHR get_native () const noexcept;
        const Window &get_window () const noexcept;

    private:
        VkSurfaceKHR _native;
        const Instance *_instance;
        const Window *_window;
    };
}

#endif /* __VKTEST_SURFACE_HPP__ */
