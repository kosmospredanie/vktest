#ifndef __VKTEST_INITALIZATION_HPP__
#define __VKTEST_INITALIZATION_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktest {
    class Initalization {
    public:
        Initalization ();
        Initalization (const Initalization &) = delete;
        Initalization (Initalization &&other) noexcept;
        ~Initalization ();
    private:
        bool _moved;
    };
}

#endif /* __VKTEST_INITALIZATION_HPP__ */
