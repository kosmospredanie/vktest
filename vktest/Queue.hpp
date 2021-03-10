#ifndef __VKTEST_QUEUE_HPP__
#define __VKTEST_QUEUE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <cstdint>
#include <vector>
#include "Device.hpp"
#include "Fence.hpp"

namespace vktest {
    class Device;
    class Fence;

    class Queue {
    public:
        Queue (const Queue &) = delete;
        VkQueue get_native () const noexcept;
        void submit (const VkSubmitInfo &submit_info, const Fence *fence) const;
        void submit (const std::vector<VkSubmitInfo> &submit_infos, const Fence *fence) const;
        /**
         * @return false if the swap chain has become incompatible with the
         * surface, true otherwise.
         */
        bool present (const VkPresentInfoKHR &present_info) const;
        void wait_idle () const noexcept;

    private:
        Queue (const Device &device, uint32_t queue_family_index, uint32_t queue_index) noexcept;

        /**
         * NOTE: VkQueue objects *cannot* be explicitly destroyed. Instead,
         * they are implicitly destroyed when the VkDevice object they are
         * retrieved from is destroyed.
         */
        VkQueue _native;

        friend class Device;
    };
}

#endif /* __VKTEST_QUEUE_HPP__ */
