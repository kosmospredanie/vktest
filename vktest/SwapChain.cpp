#include "SwapChain.hpp"
#include "QueueFamilyIndices.hpp"
#include "Window.hpp"
#include "utils.hpp"
#include <cstdint>
#include <stdexcept>

namespace vktest {
    /**
     * Chooses color and depth format
     */
    static VkSurfaceFormatKHR choose_format (const SwapChainSupport &support) {
        for (VkSurfaceFormatKHR format : support.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return support.formats[0];
    }

    /**
     * Chooses conditions for "swapping" images to the screen
     */
    static VkPresentModeKHR choose_present_mode (const SwapChainSupport &support) {
        for (VkPresentModeKHR mode : support.present_modes) {
            // Triple buffering
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
        }
        // Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /**
     * Chooses resolution of images in swap chain
     */
    static VkExtent2D choose_swap_extent (const SwapChainSupport &support, const Window &window) {
        // the special value indicating that the surface size will be determined by
        // the extent of a swapchain targeting the surface.
        if (support.capabilities.currentExtent.width != UINT32_MAX) {
            return support.capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window.get_native(), &width, &height);

            VkExtent2D actual_extent = { static_cast<uint32_t>(width),
                                         static_cast<uint32_t>(height) };
            actual_extent.width = std::max( support.capabilities.minImageExtent.width,
                                            std::min(support.capabilities.maxImageExtent.width, actual_extent.width) );
            actual_extent.height = std::max( support.capabilities.minImageExtent.height,
                                             std::min(support.capabilities.maxImageExtent.height, actual_extent.height) );
            return actual_extent;
        }
    }
}

vktest::SwapChain::SwapChain (const Device &device,
                              const Surface &surface,
                              const SwapChainSupport &support)
        : _device {&device}, _surface {&surface},
          _images {}, _image_views {},
          _framebuffers {}, _command_buffers {} {
    VkSurfaceFormatKHR format = choose_format(support);
    _image_format = format.format;
    VkPresentModeKHR present_mode = choose_present_mode(support);
    _extent = choose_swap_extent(support, surface.get_window());

    // Simply sticking to the minimum means that we may sometimes have to wait
    // on the driver to complete internal operations before we can acquire
    // another image to render to. Therefore it is recommended to request at
    // least one more image than the minimum.
    uint32_t image_count = support.capabilities.minImageCount + 1;
    // maxImageCount == 0 means that there is no maximum.
    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
        image_count = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface.get_native();
    create_info.minImageCount = image_count;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = _extent;
    // The amount of layers each image consists of. This is always 1 unless you
    // are developing a stereoscopic 3D application.
    create_info.imageArrayLayers = 1;
    // The imageUsage bit field specifies what kind of operations we'll use the
    // images in the swap chain for. To render directly to them, which means
    // that they're used as color attachment.
    //
    // It is also possible that you'll render images to a separate image first
    // to perform operations like post-processing. In that case you may use a
    // value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory
    // operation to transfer the rendered image to a swap chain image.
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices &queue_families = device.get_physical_device().get_queue_families();
    uint32_t indices[] = { queue_families.graphics.value(), queue_families.present.value() };
    if (queue_families.graphics != queue_families.present) {
        // TODO: Using the concurrent mode in this tutorial to avoid having to
        // do the ownership chapters.
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        // Concurrent mode requires you to specify in advance between which
        // queue families ownership will be shared.
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = indices;
    } else {
        // VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at
        // a time and ownership must be explicitly transferred before using it
        // in another queue family. This option offers the best performance.
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = support.capabilities.currentTransform;
    // compositeAlpha specifies if the alpha channel should be used for
    // blending with other windows in the window system.
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    // If the clipped member is set to VK_TRUE then that means that we don't
    // care about the color of pixels that are obscured, for example because
    // another window is in front of them.
    create_info.clipped = VK_TRUE;
    // TODO: Swap chain recreation
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create swap chain");

    fetch_images();
    create_image_views();
}

vktest::SwapChain::SwapChain (SwapChain &&other) noexcept
        : _image_format {other._image_format},
          _extent {std::move(other._extent)},
          _images (std::move(other._images)),
          _image_views (std::move(other._image_views)),
          _framebuffers (std::move(other._framebuffers)),
          _command_buffers (std::move(other._command_buffers)) {
    _native = other._native;
    _device = other._device;
    _surface = other._surface;
    other._native = nullptr;
}

vktest::SwapChain::~SwapChain () {
    if (_native != nullptr) vkDestroySwapchainKHR(_device->get_native(), _native, nullptr);
}

VkSwapchainKHR vktest::SwapChain::get_native () const noexcept {
    return _native;
}

const vktest::Device &vktest::SwapChain::get_device () const noexcept {
    return *_device;
}

const vktest::Surface &vktest::SwapChain::get_surface () const noexcept {
    return *_surface;
}

VkFormat vktest::SwapChain::get_image_format () const noexcept {
    return _image_format;
}

const VkExtent2D &vktest::SwapChain::get_extent () const noexcept {
    return _extent;
}

void vktest::SwapChain::create_framebuffers (const RenderPass &render_pass,
                                             const ImageView *color_image_view,
                                             const ImageView *depth_image_view) {
    _framebuffers.reserve( _images.size() );
    for (size_t i = 0; i < _images.size(); i++) {
        std::vector<const ImageView*> attachments {};
        if (color_image_view != nullptr) attachments.push_back(color_image_view);
        if (depth_image_view != nullptr) attachments.push_back(depth_image_view);
        attachments.push_back(&_image_views[i]);
        _framebuffers.emplace_back(*_device, render_pass, _extent, attachments);
    }
}

void vktest::SwapChain::create_command_buffers (const CommandPool &command_pool) {
    _command_buffers = command_pool.allocate_buffers(static_cast<uint32_t>(_images.size()), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

const std::vector<VkImage> &vktest::SwapChain::get_images () const noexcept {
    return _images;
}

const std::vector<vktest::ImageView> &vktest::SwapChain::get_image_views () const noexcept {
    return _image_views;
}

const std::vector<vktest::Framebuffer> &vktest::SwapChain::get_framebuffers () const noexcept {
    return _framebuffers;
}

std::vector<vktest::CommandBuffer*> vktest::SwapChain::get_command_buffers () noexcept {
    std::vector<CommandBuffer*> result = vktest::pvec<CommandBuffer>(_command_buffers);
    return result;
}

vktest::CommandBuffer &vktest::SwapChain::get_command_buffer (uint32_t index) noexcept {
    return _command_buffers[static_cast<size_t>(index)];
}

std::optional<uint32_t> vktest::SwapChain::acquire_next_image (uint64_t timeout,
                                                const Semaphore *semaphore,
                                                const Fence *fence) const {
    uint32_t image_index = 0;
    VkResult res = vkAcquireNextImageKHR(_device->get_native(),
                                         _native,
                                         timeout,
                                         semaphore ? semaphore->get_native() : VK_NULL_HANDLE,
                                         fence ? fence->get_native() : VK_NULL_HANDLE,
                                         &image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        return std::nullopt;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }
    return image_index;
}

void vktest::SwapChain::fetch_images () noexcept {
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(_device->get_native(), _native, &count, nullptr);
    _images.resize(count);
    vkGetSwapchainImagesKHR(_device->get_native(), _native, &count, _images.data());
}

void vktest::SwapChain::create_image_views () noexcept {
    _image_views.reserve( _images.size() );
    for (size_t i = 0; i < _images.size(); i++) {
        _image_views.emplace_back(*_device,
                _images[i], _image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}
