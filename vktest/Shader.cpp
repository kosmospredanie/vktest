#include "Shader.hpp"
#include <fstream>
#include <stdexcept>

vktest::Shader::Shader (const Device &device, const std::string &filename, ShaderDesc desc)
        : _device {&device}, _desc {std::move(desc)}, _stage_info {} {
    std::ifstream file { filename, std::ios::ate | std::ios::binary };
    if (!file.is_open()) throw std::runtime_error("Failed to open shader file");
    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer (file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    init(buffer);
}

vktest::Shader::Shader (const Device &device, const std::vector<char> &code, ShaderDesc desc)
        : _device {&device}, _desc {std::move(desc)}, _stage_info {} {
    init(code);
}

vktest::Shader::Shader (Shader &&other) noexcept
        : _desc {std::move(other._desc)}, _stage_info {std::move(other._stage_info)} {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Shader::~Shader () {
    if (_native != nullptr) vkDestroyShaderModule(_device->get_native(), _native, nullptr);
}

VkShaderModule vktest::Shader::get_native () const noexcept {
    return _native;
}

const VkPipelineShaderStageCreateInfo &vktest::Shader::get_stage_info () const noexcept {
    return _stage_info;
}

void vktest::Shader::init (const std::vector<char> &code) {
    VkShaderModuleCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkResult res = vkCreateShaderModule(_device->get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create shader module");

    _stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _stage_info.stage = _desc.stage;
    _stage_info.pName = _desc.name.c_str();
    _stage_info.module = _native;
}
