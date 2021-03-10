#include "DescriptorSet.hpp"
#include <algorithm>
#include <stdexcept>

vktest::DescriptorSet::DescriptorSet (const DescriptorPool &pool, const DescriptorSetLayout &layout) {
    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool.get_native();
    alloc_info.descriptorSetCount = 1;
    VkDescriptorSetLayout native_layout = layout.get_native();
    alloc_info.pSetLayouts = &native_layout;

    VkResult res = vkAllocateDescriptorSets(pool.get_device().get_native(), &alloc_info, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to allocate descriptor set");
}

vktest::DescriptorSet::DescriptorSet (DescriptorSet &&other) noexcept {
    _native = other._native;
    _pool = other._pool;
    other._native = nullptr;
}

vktest::DescriptorSet::~DescriptorSet () {
    if (_native != nullptr) {
        vkFreeDescriptorSets(_pool->get_device().get_native(), _pool->get_native(), 1, &_native);
        _native = nullptr;
    }
}

VkDescriptorSet vktest::DescriptorSet::get_native () const noexcept {
    return _native;
}

vktest::DescriptorSet::DescriptorSet (const DescriptorPool &pool, VkDescriptorSet native)
        : _native {native}, _pool {&pool} {
}
