#ifndef __VKTEST_IMAGEVIEW_HPP__
#define __VKTEST_IMAGEVIEW_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vktest {
    class ImageView {
    public:
        ImageView (const Device &device,
                   VkImage image,
                   VkFormat format,
                   VkImageAspectFlags aspect_flags,
                   uint32_t mip_levels);
        ImageView (const ImageView &) = delete;
        ImageView (ImageView &&other) noexcept;
        ~ImageView ();
        VkImageView get_native () const noexcept;

    private:
        VkImageView _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_IMAGEVIEW_HPP__ */
