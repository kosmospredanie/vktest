#ifndef __VKTEST_DESCRIPTORPOOL_HPP__
#define __VKTEST_DESCRIPTORPOOL_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorSet.hpp"

namespace vktest {
    class DescriptorSet;

    class DescriptorPool {
    public:
        DescriptorPool (const Device &device, uint32_t max_sets, const std::vector<VkDescriptorPoolSize> &pool_sizes);
        DescriptorPool (const DescriptorPool &) = delete;
        DescriptorPool (DescriptorPool &&other) noexcept;
        ~DescriptorPool ();
        VkDescriptorPool get_native () const noexcept;
        const Device &get_device () const noexcept;
        std::vector<DescriptorSet> allocate_descriptor_sets (const std::vector<DescriptorSetLayout*> &layouts) const;
        void free_descriptor_sets (std::vector<DescriptorSet> &sets) const noexcept;
        void free_descriptor_sets (const std::vector<DescriptorSet*> &sets) const noexcept;

    private:
        VkDescriptorPool _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_DESCRIPTORPOOL_HPP__ */
