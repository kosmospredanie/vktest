#ifndef __VKTEST_COMMANDPOOL_HPP__
#define __VKTEST_COMMANDPOOL_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"
#include "CommandBuffer.hpp"

namespace vktest {
    class CommandBuffer;

    class CommandPool {
    public:
        CommandPool (const Device &device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
        CommandPool (const CommandPool &) = delete;
        CommandPool (CommandPool &&other) noexcept;
        ~CommandPool ();
        VkCommandPool get_native () const noexcept;
        const Device &get_device () const noexcept;
        std::vector<CommandBuffer> allocate_buffers (uint32_t count, VkCommandBufferLevel level) const;
        void free_buffers (std::vector<CommandBuffer> &buffers) const noexcept;
        void free_buffers (const std::vector<CommandBuffer*> &buffers) const noexcept;

    private:
        VkCommandPool _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_COMMANDPOOL_HPP__ */
