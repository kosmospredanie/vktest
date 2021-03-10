#ifndef __VKTEST_FRAMEBUFFER_HPP__
#define __VKTEST_FRAMEBUFFER_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Device.hpp"
#include "RenderPass.hpp"
#include "ImageView.hpp"

namespace vktest {
    class Framebuffer {
    public:
        Framebuffer (const Device &device,
                     const RenderPass &render_pass,
                     const VkExtent2D &size,
                     const std::vector<const ImageView*> &attachments);
        Framebuffer (const Framebuffer &) = delete;
        Framebuffer (Framebuffer &&other) noexcept;
        ~Framebuffer ();
        VkFramebuffer get_native () const noexcept;
        const Device &get_device () const noexcept;

    private:
        VkFramebuffer _native;
        const Device *_device;
    };
}

#endif /* __VKTEST_FRAMEBUFFER_HPP__ */
