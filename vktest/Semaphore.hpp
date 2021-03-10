#ifndef __VKTEST_SEMAPHORE_HPP__
#define __VKTEST_SEMAPHORE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    class Semaphore {
    public:
        Semaphore (const Device &device);
        Semaphore (const Semaphore &) = delete;
        Semaphore (Semaphore &&other) noexcept;
        ~Semaphore ();
        VkSemaphore get_native () const noexcept;

    private:
        VkSemaphore _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_SEMAPHORE_HPP__ */
