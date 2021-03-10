#ifndef __VKTEST_SWAPCHAIN_HPP__
#define __VKTEST_SWAPCHAIN_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <optional>
#include "SwapChainSupport.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "ImageView.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"

namespace vktest {
    class SwapChain {
    public:
        SwapChain (const Device &device,
                   const Surface &surface,
                   const SwapChainSupport &support);
        SwapChain (const SwapChain &) = delete;
        SwapChain (SwapChain &&other) noexcept;
        ~SwapChain ();
        VkSwapchainKHR get_native () const noexcept;
        const Device &get_device () const noexcept;
        const Surface &get_surface () const noexcept;
        VkFormat get_image_format () const noexcept;
        const VkExtent2D &get_extent () const noexcept;
        void create_framebuffers (const RenderPass &render_pass,
                                  const ImageView *color_image_view,
                                  const ImageView *depth_image_view);
        void create_command_buffers (const CommandPool &command_pool);
        const std::vector<VkImage> &get_images () const noexcept;
        const std::vector<ImageView> &get_image_views () const noexcept;
        const std::vector<Framebuffer> &get_framebuffers () const noexcept;
        std::vector<CommandBuffer*> get_command_buffers () noexcept;
        CommandBuffer &get_command_buffer (uint32_t index) noexcept;

        /**
         * Acquires an image from the swap chain.
         *
         * @return The index of the next image to use, or empty if the swap
         * chain has become incompatible with the surface.
         */
        std::optional<uint32_t> acquire_next_image (uint64_t timeout, const Semaphore *semaphore, const Fence *fence) const;

    private:
        void fetch_images () noexcept;
        void create_image_views () noexcept;

        VkSwapchainKHR _native;
        const Device *_device;
        const Surface *_surface;
        VkFormat _image_format;
        VkExtent2D _extent;
        std::vector<VkImage> _images;
        std::vector<ImageView> _image_views;
        std::vector<Framebuffer> _framebuffers;
        std::vector<CommandBuffer> _command_buffers;
    };
}

#endif /* __VKTEST_SWAPCHAIN_HPP__ */
