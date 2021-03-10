#ifndef __VKTEST_PIPELINELAYOUT_HPP__
#define __VKTEST_PIPELINELAYOUT_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"
#include "DescriptorSetLayout.hpp"

namespace vktest {
    class PipelineLayout {
    public:
        PipelineLayout (const Device &device, const std::vector<DescriptorSetLayout*> &descriptor_set_layouts);
        PipelineLayout (const PipelineLayout &) = delete;
        PipelineLayout (PipelineLayout &&other) noexcept;
        ~PipelineLayout ();
        VkPipelineLayout get_native () const noexcept;
        const Device &get_device () const noexcept;

    private:
        VkPipelineLayout _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_PIPELINELAYOUT_HPP__ */
