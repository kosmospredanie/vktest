#include "PipelineLayout.hpp"
#include <algorithm>
#include <stdexcept>

vktest::PipelineLayout::PipelineLayout (
        const Device &device,
        const std::vector<DescriptorSetLayout*> &descriptor_set_layouts) : _device {&device} {
    std::vector<VkDescriptorSetLayout> native_desc_set_layouts (descriptor_set_layouts.size());
    std::transform(descriptor_set_layouts.begin(), descriptor_set_layouts.end(), native_desc_set_layouts.begin(),
            [](const DescriptorSetLayout *desc_set_layout) { return desc_set_layout->get_native(); } );

    VkPipelineLayoutCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = static_cast<uint32_t>( native_desc_set_layouts.size() ); // Optional
    create_info.pSetLayouts = native_desc_set_layouts.data(); // Optional
    create_info.pushConstantRangeCount = 0; // Optional
    create_info.pPushConstantRanges = nullptr; // Optional

    VkResult res = vkCreatePipelineLayout(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create pipeline layout");
}

vktest::PipelineLayout::PipelineLayout (PipelineLayout &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::PipelineLayout::~PipelineLayout () {
    if (_native != nullptr) vkDestroyPipelineLayout(_device->get_native(), _native, nullptr);
}

VkPipelineLayout vktest::PipelineLayout::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::PipelineLayout::get_device () const noexcept {
    return *_device;
}
