#include "Sampler.hpp"
#include <stdexcept>

vktest::Sampler::Sampler (const Device &device,
                        VkFilter mag_filter,
                        VkFilter min_filter,
                        const AddressModes &address_modes,
                        float max_anisotropy,
                        uint32_t mip_levels) : _device {&device} {
    VkSamplerCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = mag_filter;
    create_info.minFilter = min_filter;

    create_info.addressModeU = address_modes.u;
    create_info.addressModeV = address_modes.v;
    create_info.addressModeW = address_modes.w;

    create_info.anisotropyEnable = max_anisotropy >= 1.0f ? VK_TRUE : VK_FALSE;
    create_info.maxAnisotropy = max_anisotropy;

    create_info.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;

    // unnormalizedCoordinates:
    //  * VK_TRUE: [0, width)
    //  * VK_FALSE: [0, 1)
    create_info.unnormalizedCoordinates = VK_FALSE;

    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;

    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0f;
    create_info.maxLod = static_cast<float>(mip_levels);

    VkResult res = vkCreateSampler(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create texture sampler");
}

vktest::Sampler::Sampler (Sampler &&other) noexcept {
    _native = other._native;
    _device = other._device;
    other._native = nullptr;
}

vktest::Sampler::~Sampler () {
    if (_native != nullptr) vkDestroySampler(_device->get_native(), _native, nullptr);
}

VkSampler vktest::Sampler::get_native () const noexcept {
    return _native;
}
