#ifndef __VKTEST_DESCRIPTORSETLAYOUT_HPP__
#define __VKTEST_DESCRIPTORSETLAYOUT_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"

namespace vktest {
    class DescriptorSetLayout {
    public:
        DescriptorSetLayout (const Device &device, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
        DescriptorSetLayout (const DescriptorSetLayout &) = delete;
        DescriptorSetLayout (DescriptorSetLayout &&other) noexcept;
        ~DescriptorSetLayout ();
        VkDescriptorSetLayout get_native () const noexcept;

    private:
        VkDescriptorSetLayout _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_DESCRIPTORSETLAYOUT_HPP__ */
