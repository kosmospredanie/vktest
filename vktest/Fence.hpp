#ifndef __VKTEST_FENCE_HPP__
#define __VKTEST_FENCE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    class Device;

    class Fence {
    public:
        Fence (const Device &device, VkFenceCreateFlags flags);
        Fence (const Fence &) = delete;
        Fence (Fence &&other) noexcept;
        ~Fence ();
        VkFence get_native () const noexcept;
        void wait (uint64_t timeout) const noexcept;
        /* Resets the fence.
         *
         * NOTE: Unlike the semaphores, we manually need to restore the fence to
         * the unsignaled state by resetting it.
         */
        void reset () const noexcept;

    private:
        VkFence _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_FENCE_HPP__ */
