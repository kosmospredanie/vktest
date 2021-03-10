#ifndef __VKTEST_QUEUEFAMILYINDICES_HPP__
#define __VKTEST_QUEUEFAMILYINDICES_HPP__

#include <optional>
#include <cstdint>

namespace vktest {
    struct QueueFamilyIndices {
        bool is_complete () const noexcept;
        // NOTE: Any queue family with VK_QUEUE_GRAPHICS_BIT or
        // VK_QUEUE_COMPUTE_BIT capabilities already implicitly support
        // VK_QUEUE_TRANSFER_BIT operations.
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
        std::optional<uint32_t> transfer;
    };
}

#endif /* __VKTEST_QUEUEFAMILYINDICES_HPP__ */
