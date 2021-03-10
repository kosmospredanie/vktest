#ifndef __VKTEST_DEVICE_HPP__
#define __VKTEST_DEVICE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include <vector>
#include <memory>
#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "Fence.hpp"
#include "DeviceMemory.hpp"

namespace vktest {
    class PhysicalDevice;
    class Queue;
    class Fence;
    class DeviceMemory;

    struct QueueCreateDesc {
        QueueCreateDesc () : family_index {0}, count {0}, priorities {} {}
        QueueCreateDesc (uint32_t family_index,
                         uint32_t count,
                         float priority) : family_index {family_index},
                                           count {count},
                                           priorities {priority} {}

        uint32_t family_index;
        uint32_t count;
        std::vector<float> priorities;
    };

    /**
     * A logical device
     */
    class Device {
    public:
        Device (const PhysicalDevice &physical_device, const std::vector<QueueCreateDesc> &queue_create_descs);
        Device (const Device &) = delete;
        Device (Device &&other) noexcept;
        ~Device ();
        VkDevice get_native () const noexcept;
        const PhysicalDevice &get_physical_device () const noexcept;
        Queue &get_queue (uint32_t queue_family_index, uint32_t queue_index);
        void wait_idle () const noexcept;
        void wait_for_fences (const std::vector<const Fence*> fences, bool wait_all, uint64_t timeout) const noexcept;
        void reset_fences (const std::vector<const Fence*> fences) const noexcept;

    private:
        void fetch_queues (const std::vector<QueueCreateDesc> &queue_create_descs) noexcept;

        /**
         * NOTE: VkDevice objects *can* be destroyed when all VkQueue objects
         * retrieved from them are idle, and all objects created from them have
         * been destroyed; VkFence, VkSemaphore, VkEvent, VkBuffer, etc.
         */
        VkDevice _native;
        const PhysicalDevice *_physical_device;
        std::map<std::pair<uint32_t,uint32_t>,std::unique_ptr<Queue>> _queues;
    };
}

#endif /* __VKTEST_DEVICE_HPP__ */
