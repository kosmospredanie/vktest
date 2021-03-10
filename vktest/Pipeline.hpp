#ifndef __VKTEST_PIPELINE_HPP__
#define __VKTEST_PIPELINE_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "PipelineLayout.hpp"
#include "RenderPass.hpp"

namespace vktest {
    class Pipeline {
    public:
        Pipeline (const PipelineLayout &layout,
                  const VkViewport &viewport,
                  const VkRect2D &scissor,
                  const std::vector<VkPipelineShaderStageCreateInfo> &stages,
                  const RenderPass &render_pass,
                  VkSampleCountFlagBits msaa_samples);
        Pipeline (const Pipeline &) = delete;
        Pipeline (Pipeline &&other) noexcept;
        ~Pipeline ();
        VkPipeline get_native () const noexcept;
        const PipelineLayout &get_layout () const noexcept;

    private:
        VkPipeline _native;
        const PipelineLayout *_layout;
    };
}

#endif /* __VKTEST_PIPELINE_HPP__ */
