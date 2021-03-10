#include "DescriptorSetLayout.hpp"
#include <stdexcept>

vktest::DescriptorSetLayout::DescriptorSetLayout (
        const Device &device, const std::vector<VkDescriptorSetLayoutBinding> &bindings)
        : _device {&device} {
    VkDescriptorSetLayoutCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create descriptor set layout");
}

vktest::DescriptorSetLayout::DescriptorSetLayout (DescriptorSetLayout &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::DescriptorSetLayout::~DescriptorSetLayout () {
    if (_native != nullptr) vkDestroyDescriptorSetLayout(_device->get_native(), _native, nullptr);
}

VkDescriptorSetLayout vktest::DescriptorSetLayout::get_native () const noexcept {
    return _native;
}
