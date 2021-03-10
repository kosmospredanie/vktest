#ifndef __VKTEST_DESCRIPTORSET_HPP__
#define __VKTEST_DESCRIPTORSET_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

namespace vktest {
    class DescriptorPool;

    class DescriptorSet {
    public:
        DescriptorSet (const DescriptorPool &pool, const DescriptorSetLayout &layout);
        DescriptorSet (const DescriptorSet &) = delete;
        DescriptorSet (DescriptorSet &&other) noexcept;
        ~DescriptorSet ();
        VkDescriptorSet get_native () const noexcept;

    private:
        DescriptorSet (const DescriptorPool &pool, VkDescriptorSet _native);

        // When a pool is destroyed, all descriptor sets allocated from the
        // pool are freed.
        VkDescriptorSet _native;
        const DescriptorPool *_pool;

        friend class DescriptorPool;
    };
}

#endif /* __VKTEST_DESCRIPTORSET_HPP__ */
