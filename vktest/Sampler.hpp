#ifndef __VKTEST_Sampler_HPP__
#define __VKTEST_Sampler_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    struct AddressModes {
        VkSamplerAddressMode u;
        VkSamplerAddressMode v;
        VkSamplerAddressMode w;
    };

    class Sampler {
    public:
        Sampler (const Device &device,
                 VkFilter mag_filter,
                 VkFilter min_filter,
                 const AddressModes &address_modes,
                 float max_anisotropy,
                 uint32_t mip_levels);
        Sampler (const Sampler &) = delete;
        Sampler (Sampler &&other) noexcept;
        ~Sampler ();
        VkSampler get_native () const noexcept;

    private:
        VkSampler _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_Sampler_HPP__ */
