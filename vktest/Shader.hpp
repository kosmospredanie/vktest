#ifndef __VKTEST_SHADER_HPP__
#define __VKTEST_SHADER_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <Device.hpp>

namespace vktest {
    struct ShaderDesc {
        VkShaderStageFlagBits stage;
        std::string name;
    };

    class Shader {
    public:
        Shader (const Device &device, const std::string &filename, ShaderDesc desc);
        Shader (const Device &device, const std::vector<char> &code, ShaderDesc desc);
        Shader (const Shader &) = delete;
        Shader (Shader &&other) noexcept;
        ~Shader ();
        VkShaderModule get_native () const noexcept;
        const VkPipelineShaderStageCreateInfo &get_stage_info () const noexcept;

    private:
        void init (const std::vector<char> &code);

        VkShaderModule _native;
        const Device *_device;
        const ShaderDesc _desc;
        VkPipelineShaderStageCreateInfo _stage_info;
    };
}

#endif /* __VKTEST_SHADER_HPP__ */
