#include "DescriptorPool.hpp"
#include <stdexcept>
#include <algorithm>

vktest::DescriptorPool::DescriptorPool (
        const Device &device, uint32_t max_sets, const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : _device {&device}{
    VkDescriptorPoolCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT: Individual descriptor
    // sets can be freed using vkFreeDescriptorSets.
    create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    create_info.pPoolSizes = pool_sizes.data();

    VkResult res = vkCreateDescriptorPool(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create descriptor pool");
}

vktest::DescriptorPool::DescriptorPool (DescriptorPool &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::DescriptorPool::~DescriptorPool () {
    if (_native != nullptr) vkDestroyDescriptorPool(_device->get_native(), _native, nullptr);
}

VkDescriptorPool vktest::DescriptorPool::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::DescriptorPool::get_device () const noexcept {
    return *_device;
}

std::vector<vktest::DescriptorSet> vktest::DescriptorPool::allocate_descriptor_sets (
        const std::vector<DescriptorSetLayout*> &layouts) const {
    std::vector<VkDescriptorSetLayout> native_layouts (layouts.size());
    std::transform(layouts.begin(), layouts.end(), native_layouts.begin(), [](const DescriptorSetLayout *layout) {
        return layout->get_native();
    });

    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = _native;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(native_layouts.size());
    alloc_info.pSetLayouts = native_layouts.data();

    std::vector<VkDescriptorSet> native_sets (native_layouts.size());
    VkResult res = vkAllocateDescriptorSets(_device->get_native(), &alloc_info, native_sets.data());
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to allocate descriptor sets");

    std::vector<DescriptorSet> sets {};
    sets.reserve(native_sets.size());
    for (VkDescriptorSet native_set : native_sets) {
        DescriptorSet set { *this, native_set };
        sets.push_back( std::move(set) );
    }
    return sets;
}

void vktest::DescriptorPool::free_descriptor_sets (std::vector<DescriptorSet> &sets) const noexcept {
    std::vector<VkDescriptorSet> native_sets {};
    native_sets.reserve(sets.size());
    for (DescriptorSet &set : sets) {
        native_sets.push_back(set._native);
        set._native = nullptr;
    }
    vkFreeDescriptorSets(_device->get_native(), _native, static_cast<uint32_t>(native_sets.size()), native_sets.data());
}

void vktest::DescriptorPool::free_descriptor_sets (const std::vector<DescriptorSet*> &sets) const noexcept {
    std::vector<VkDescriptorSet> native_sets {};
    for (DescriptorSet *set : sets) {
        if (set->_native != nullptr) {
            native_sets.push_back(set->_native);
            set->_native = nullptr;
        }
    }
    vkFreeDescriptorSets(_device->get_native(), _native, static_cast<uint32_t>(native_sets.size()), native_sets.data());
}
