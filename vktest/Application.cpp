#include "Application.hpp"
#include "config.hpp"
#include "UniformBufferObject.hpp"
#include <stdexcept>
#include <cstring>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

vktest::Application::Application (std::string app_name)
        : _app_name { std::move(app_name) },
          _uniform_buffer_memories {},
          _uniform_buffers {},
          _descriptor_sets {},
          _image_available_semaphores {},
          _render_finished_semaphores {},
          _in_flight_fences {},
          _images_in_flight {},
          _current_frame {0},
          _framebuffer_resized {false} {
    init();
    loop();
}

void vktest::Application::init () {
    init_window();
    init_vulkan();
}

void vktest::Application::loop () {
    while ( !glfwWindowShouldClose(_window->get_native()) ) {
        glfwPollEvents();
        update();
    }
    _device->wait_idle();
}

void vktest::Application::update () {
    draw();
}

void vktest::Application::init_window () {
    _init = std::make_unique<Initalization>();
    _window = std::make_unique<Window>(WIDTH, HEIGHT, _app_name);
    _window->set_user_pointer(&_framebuffer_resized);
    _window->set_framebuffer_resize_callback(on_framebuffer_resize);
}

void vktest::Application::on_framebuffer_resize (GLFWwindow *window, int width, int height) {
    bool *resized = static_cast<bool*>(glfwGetWindowUserPointer(window));
    *resized = true;
}

void vktest::Application::init_vulkan () {
    _instance = std::make_unique<Instance>( _app_name.c_str() );
    _surface = std::make_unique<Surface>(*_instance, *_window);

    _instance->select_physical_device(*_surface);
    _physical_device = &(_instance->get_physical_device());
    _msaa_samples = get_max_usable_sample_count();
    uint32_t graphics_queue_family = _physical_device->get_queue_families().graphics.value();
    uint32_t present_queue_family = _physical_device->get_queue_families().present.value();
    // uint32_t transfer_queue_family = _physical_device->get_queue_families().transfer.value();

    std::vector<QueueCreateDesc> queue_create_descs {
        {graphics_queue_family, 1, 1.0f}, {present_queue_family, 1, 1.0f} // , {transfer_queue_family, 1, 1.0f}
    };
    _device = std::make_unique<Device>(*_physical_device, queue_create_descs);
    _graphics_queue = &(_device->get_queue(graphics_queue_family, 0));
    _present_queue = &(_device->get_queue(present_queue_family, 0));

    _command_pool = std::make_unique<CommandPool>(*_device, graphics_queue_family, 0);
    create_swap_chain();

    _vert_shader = std::make_unique<Shader>(*_device, "data/shader.vert.spv", (ShaderDesc) { VK_SHADER_STAGE_VERTEX_BIT, "main" });
    _frag_shader = std::make_unique<Shader>(*_device, "data/shader.frag.spv", (ShaderDesc) { VK_SHADER_STAGE_FRAGMENT_BIT, "main" });

    create_render_pass();
    create_descriptor_set_layout();
    create_pipeline();

    create_color_resources();
    create_depth_resources();
    create_framebuffers();
    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();
    load_model();
    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
    record_command_buffers();
    create_sync_objects();
}

void vktest::Application::create_swap_chain () {
    SwapChainSupport swap_chain_support = _physical_device->query_swap_chain_support(*_surface);
    _swap_chain = std::make_unique<SwapChain>(*_device, *_surface, swap_chain_support);
}

void vktest::Application::create_render_pass () {
    std::vector<VkSubpassDependency> dependencies = prepare_subpass_dependencies();
    _render_pass = std::make_unique<RenderPass>(*_device,
                                                _swap_chain->get_image_format(),
                                                find_depth_format(),
                                                _msaa_samples,
                                                dependencies);
}

std::vector<VkSubpassDependency> vktest::Application::prepare_subpass_dependencies () const noexcept {
    VkSubpassDependency dependency {};
    // The first two fields specify the indices of the dependency and the
    // dependent subpass. The special value VK_SUBPASS_EXTERNAL refers to the
    // implicit subpass before or after the render pass depending on whether it
    // is specified in srcSubpass or dstSubpass. The dstSubpass must always be
    // higher than srcSubpass to prevent cycles in the dependency graph (unless
    // one of the subpasses is VK_SUBPASS_EXTERNAL).
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    // The two fields specify the operations to wait on and the stages in which
    // these operations occur. We need to wait for the swap chain to finish
    // reading from the image before we can access it.
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    // The operations that should wait on this are in the color attachment
    // stage and involve the writing of the color attachment. These settings
    // will prevent the transition from happening until it's actually necessary
    // (and allowed): when we want to start writing colors to it.
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    std::vector<VkSubpassDependency> dependencies {dependency};
    return dependencies;
}

void vktest::Application::create_descriptor_set_layout () {
    VkDescriptorSetLayoutBinding ubo_layout_binding {};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding sampler_layout_binding {};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    // We intend to use the combined image sampler descriptor in the fragment
    // shader. That's where the color of the fragment is going to be determined.
    // It is possible to use texture sampling in the vertex shader, for example
    // to dynamically deform a grid of vertices by a *heightmap*.
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = nullptr; // Optional

    std::vector<VkDescriptorSetLayoutBinding> bindings { ubo_layout_binding, sampler_layout_binding };
    _descriptor_set_layout = std::make_unique<DescriptorSetLayout>(*_device, bindings);
}

void vktest::Application::create_pipeline () {
    std::vector<DescriptorSetLayout*> desc_set_layouts { _descriptor_set_layout.get() };
    _pipeline_layout = std::make_unique<PipelineLayout>(*_device, desc_set_layouts);

    VkViewport viewport { 0.0f, 0.0f, (float) _swap_chain->get_extent().width, (float) _swap_chain->get_extent().height, 0.0f, 1.0f };
    VkRect2D scissor { {0, 0}, _swap_chain->get_extent() };
    std::vector<VkPipelineShaderStageCreateInfo> stages { _vert_shader->get_stage_info(), _frag_shader->get_stage_info() };
    _pipeline = std::make_unique<Pipeline>(*_pipeline_layout, viewport, scissor, stages, *_render_pass, _msaa_samples);
}

void vktest::Application::create_framebuffers () {
    _swap_chain->create_framebuffers(*_render_pass, _color_image_view.get(), _depth_image_view.get());
}

void vktest::Application::create_color_resources () {
    VkFormat color_format = _swap_chain->get_image_format();
    const VkExtent2D &extent = _swap_chain->get_extent();

    std::tie(_color_image, _color_image_memory) = create_image(
            extent.width, extent.height,
            1, _msaa_samples, color_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _color_image_view = std::make_unique<ImageView>(*_device,
            _color_image->get_native(), color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void vktest::Application::create_depth_resources () {
    VkFormat depth_format = find_depth_format();
    const VkExtent2D &extent = _swap_chain->get_extent();
    std::tie(_depth_image, _dpeth_image_memory) = create_image(
            extent.width, extent.height,
            1, _msaa_samples, depth_format,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    _depth_image_view = std::make_unique<ImageView>(*_device,
            _depth_image->get_native(), depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    transition_image_layout(*_depth_image, depth_format,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat vktest::Application::find_depth_format () const {
    return find_supported_format(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool vktest::Application::has_stencil_component (VkFormat format) const noexcept {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat vktest::Application::find_supported_format (
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) const {
    for (VkFormat format : candidates) {
        VkFormatProperties props = _physical_device->get_format_properties(format);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format");
}

void vktest::Application::create_texture_image () {
    int width, height, channels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH, &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) throw std::runtime_error("Failed to load texture image");
    VkDeviceSize image_size = width * height * 4;
    // log2 -> calculates how many times that dimension can be divided by 2.
    _mip_levels = static_cast<uint32_t>(std::floor( std::log2(std::max(width, height)) ) + 1);

    std::unique_ptr<Buffer> staging_buffer;
    std::unique_ptr<DeviceMemory> staging_buffer_memory;
    std::tie(staging_buffer, staging_buffer_memory) = create_buffer(
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = staging_buffer_memory->map(0, image_size);
    std::memcpy(data, pixels, static_cast<size_t>(image_size));
    staging_buffer_memory->unmap();
    stbi_image_free(pixels);

    std::tie(_texture_image, _texture_image_memory) = create_image(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            _mip_levels, VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transition_image_layout(*_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mip_levels);
        copy_buffer_to_image(*staging_buffer, *_texture_image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    // Transitioned to *VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL* while generating mipmaps.
    // transition_image_layout(*_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    generate_mipmaps(*_texture_image, VK_FORMAT_R8G8B8A8_SRGB, width, height, _mip_levels);
}

void vktest::Application::generate_mipmaps (const Image &image,
                                            VkFormat image_format,
                                            int32_t width,
                                            int32_t height,
                                            uint32_t mip_levels) const {
    /* It should be noted that it is uncommon in practice to generate the
     * mipmap levels at runtime anyway. Usually they are pregenerated and
     * stored in the texture file alongside the base level to improve loading
     * speed.
     */

    // Check if image format supports linear blitting
    VkFormatProperties format_props = _physical_device->get_format_properties(image_format);
    if ( !(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ) {
        // In this case, you could implement the mipmap generation in software
        // with a library like stb_image_resize.
        throw std::runtime_error("Texture image format does not support linear blitting");
    }

    CommandBuffer cmdbuf = begin_single_time_commands();

    std::vector<VkImageMemoryBarrier> barriers (1);
    VkImageMemoryBarrier &barrier = barriers[0];
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image.get_native();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width = width;
    int32_t mip_height = height;

    for (uint32_t i = 1;i < mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        cmdbuf.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, barriers);

        // Region(s) that will be used in the blit operation.
        std::vector<VkImageBlit> blits (1);
        VkImageBlit &blit = blits[0];
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mip_width, mip_height, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        // NOTE: Beware if you are using a dedicated transfer queue:
        // *vkCmdBlitImage* must be submitted to a queue with graphics capability.
        cmdbuf.blit_image(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          blits, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        // This transition waits on the current blit command to finish. All
        // sampling operations will wait on this transition to finish.
        cmdbuf.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, barriers);

        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    // This barrier transitions the last mip level from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. This wasn't handled by the loop.
    cmdbuf.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, barriers);

    end_single_time_commands( std::move(cmdbuf) );
}

std::pair<std::unique_ptr<vktest::Image>,std::unique_ptr<vktest::DeviceMemory>>
vktest::Application::create_image (
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels,
        VkSampleCountFlagBits num_samples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties) const {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    // It is possible that the format is not supported by the graphics hardware.
    image_info.format = format;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1;
    // VK_IMAGE_TILING_LINEAR, VK_IMAGE_TILING_OPTIMAL
    image_info.tiling = tiling;
    // initialLayout:
    //  * VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
    //  * VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // usage:
    //  * VK_IMAGE_USAGE_TRANSFER_DST_BIT: The image is going to be used as destination for the buffer copy.
    //  * VK_IMAGE_USAGE_SAMPLED_BIT: We want to be able to access the image from the shader to color our mesh.
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // This is only relevant for images that will be used as attachments.
    image_info.samples = num_samples;
    // flags: e.g. sparse images
    image_info.flags = 0; // Optional

    auto texture_image = std::make_unique<Image>(*_device, image_info);
    VkMemoryRequirements mem_reqs = texture_image->get_memory_requirements();
    uint32_t memory_type_index = find_memory_type(mem_reqs.memoryTypeBits, properties);
    auto texture_image_memory = std::make_unique<DeviceMemory>(*_device, mem_reqs.size, memory_type_index);
    texture_image->bind_memory(*texture_image_memory, 0);

    auto pair = std::make_pair(std::move(texture_image), std::move(texture_image_memory));
    return pair;
}

void vktest::Application::transition_image_layout (
        const Image &image,
        VkFormat format,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        uint32_t mip_levels) const {
    CommandBuffer cmdbuf = begin_single_time_commands();

    std::vector<VkImageMemoryBarrier> barriers (1);
    VkImageMemoryBarrier &barrier = barriers[0];
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // It is possible to use *VK_IMAGE_LAYOUT_UNDEFINED* as oldLayout if you
    // don't care about the existing contents of the image.
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    // If you are using the barrier to transfer queue family ownership, then
    // these two fields should be the indices of the queue families. They must
    // be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this.
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    // The image and subresourceRange specify the image that is affected and
    // the specific part of the image.
    barrier.image = image.get_native();

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (has_stencil_component(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    // The number of mipmap levels (starting from baseMipLevel) accessible to the view.
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        // Command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT
        // synchronization at the beginning.
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        // The reading happens in the VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        // stage and the writing in the VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT.
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition");
    }

    cmdbuf.pipeline_barrier(source_stage, destination_stage, 0, barriers);
    end_single_time_commands( std::move(cmdbuf) );
}

void vktest::Application::copy_buffer_to_image (
        const Buffer &buffer,
        const Image &image,
        uint32_t width,
        uint32_t height) const {
    CommandBuffer cmdbuf = begin_single_time_commands();

    std::vector<VkBufferImageCopy> regions (1);
    VkBufferImageCopy &region = regions[0];
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    cmdbuf.copy_buffer(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions);
    end_single_time_commands( std::move(cmdbuf) );
}

VkSampleCountFlagBits vktest::Application::get_max_usable_sample_count () const noexcept {
    VkPhysicalDeviceProperties props = _physical_device->get_properties();

    VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

void vktest::Application::create_texture_image_view () {
    _texture_image_view = std::make_unique<ImageView>(*_device,
            _texture_image->get_native(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, _mip_levels);
}

void vktest::Application::create_texture_sampler () {
    AddressModes address_modes { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
    _texture_sampler = std::make_unique<Sampler>(*_device,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR,
            address_modes,
            _physical_device->get_properties().limits.maxSamplerAnisotropy,
            _mip_levels);
}

void vktest::Application::load_model () {
    /* An OBJ file consists of positions, normals, texture coordinates and
     * faces. Faces consist of an arbitrary amount of vertices, where each
     * vertex refers to a position, normal and/or texture coordinate by index.
     */

    // attrib: Holds all of the positions, normals and texture coordinates in
    // its attrib.vertices, attrib.normals and attrib.texcoords vectors.
    tinyobj::attrib_t attrib;
    // shapes: contains all of the separate objects and their faces.
    std::vector<tinyobj::shape_t> shapes;
    // a material and texture per face, but we will be ignoring those.
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Faces in OBJ files can actually contain an arbitrary number of vertices,
    // whereas our application can only render triangles. Luckily the LoadObj
    // has an optional parameter to automatically triangulate such faces, which
    // is enabled by default.
    bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH);
    if (!res) throw std::runtime_error(warn + err);

    // Keep track of the unique vertices and respective indices.
    std::unordered_map<Vertex, uint32_t> unique_vertices {};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex {};
            vertex.color = {1.0f, 1.0f, 1.0f};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            // The OBJ format assumes a coordinate system where a vertical
            // coordinate of 0 means the bottom of the image, however we've
            // uploaded our image into Vulkan in a top to bottom orientation
            // where 0 means the top of the image.
            vertex.tex_coord = {
                attrib.texcoords[2 * index.texcoord_index],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(unique_vertices[vertex]);
        }
    }
}

void vktest::Application::create_vertex_buffer () {
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

    // Staging buffer: A host visible buffer as temporary buffer.
    std::unique_ptr<Buffer> staging_buffer;
    std::unique_ptr<DeviceMemory> staging_buffer_memory;
    std::tie(staging_buffer, staging_buffer_memory) = create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = staging_buffer_memory->map(0, buffer_size);
    std::memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    staging_buffer_memory->unmap();

    // A device local one as actual vertex buffer.
    // Device local buffer: That we're not able to use vkMapMemory. However, we
    // can copy data from the staging buffer to the device local buffer.
    std::tie(_vertex_buffer, _vertex_buffer_memory) = create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // Move the vertex data to the device local buffer.
    copy_buffer(*staging_buffer, *_vertex_buffer, buffer_size);
}

void vktest::Application::create_index_buffer () {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    // Staging buffer: A host visible buffer as temporary buffer.
    std::unique_ptr<Buffer> staging_buffer;
    std::unique_ptr<DeviceMemory> staging_buffer_memory;
    std::tie(staging_buffer, staging_buffer_memory) = create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = staging_buffer_memory->map(0, buffer_size);
    std::memcpy(data, indices.data(), (size_t) buffer_size);
    staging_buffer_memory->unmap();

    // A device local one as actual vertex buffer.
    // Device local buffer: That we're not able to use vkMapMemory. However, we
    // can copy data from the staging buffer to the device local buffer.
    std::tie(_index_buffer, _index_buffer_memory) = create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // Move the vertex data to the device local buffer.
    copy_buffer(*staging_buffer, *_index_buffer, buffer_size);
}

void vktest::Application::create_descriptor_pool () {
    std::vector<VkDescriptorPoolSize> pool_sizes (2);
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>( _swap_chain->get_images().size() );
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>( _swap_chain->get_images().size() );

    uint32_t max_sets = static_cast<uint32_t>( _swap_chain->get_images().size() );
    _descriptor_pool = std::make_unique<DescriptorPool>(*_device, max_sets, pool_sizes);
}

void vktest::Application::create_descriptor_sets () {
    std::vector<DescriptorSetLayout*> layouts (_swap_chain->get_images().size(), _descriptor_set_layout.get());
    _descriptor_sets = _descriptor_pool->allocate_descriptor_sets(layouts);

    for (size_t i = 0; i < _swap_chain->get_images().size(); i++) {
        // Specifies the buffer that the descriptors refer, and the region
        // within it that contains the data for the descriptor.
        VkDescriptorBufferInfo buffer_info {};
        buffer_info.buffer = _uniform_buffers[i]->get_native();
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject); // VK_WHOLE_SIZE is also possible

        VkDescriptorImageInfo image_info {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = _texture_image_view->get_native();
        image_info.sampler = _texture_sampler->get_native();

        // The configuration of descriptors
        std::vector<VkWriteDescriptorSet> descriptor_writes (2);

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = _descriptor_sets[i].get_native();
        descriptor_writes[0].dstBinding = 0;
        // The first index in the array that we want to update.
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        // An array with descriptorCount structs. Other type fields are ignored.
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = nullptr; // Optional
        descriptor_writes[0].pTexelBufferView = nullptr; // Optional

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = _descriptor_sets[i].get_native();
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        // vkUpdateDescriptorSets accepts two kinds of arrays as parameters:
        // an array of *VkWriteDescriptorSet* and an array of
        // *VkCopyDescriptorSet*. The latter can be used to copy descriptors to
        // each other.
        vkUpdateDescriptorSets(_device->get_native(),
                               static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(),
                               0, nullptr);
    }
}

void vktest::Application::create_uniform_buffers () {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    _uniform_buffers.reserve( _swap_chain->get_images().size() );
    _uniform_buffer_memories.reserve( _swap_chain->get_images().size() );

    for (size_t i = 0; i < _swap_chain->get_images().size(); i++) {
        auto pair = create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        _uniform_buffers.push_back( std::move(pair.first) );
        _uniform_buffer_memories.push_back( std::move(pair.second) );
    }
}

std::pair< std::unique_ptr<vktest::Buffer>, std::unique_ptr<vktest::DeviceMemory> >
vktest::Application::create_buffer (VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
    auto buffer = std::make_unique<Buffer>(*_device, size, usage, VK_SHARING_MODE_EXCLUSIVE);
    // NOTE: It should be noted that in a real world application, you're not
    // supposed to actually call vkAllocateMemory for every individual buffer.
    // The maximum number of simultaneous memory allocations is limited by the
    // *maxMemoryAllocationCount* physical device limit, which may be as low as
    // 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to
    // allocate memory for a large number of objects at the same time is to
    // create a custom allocator that splits up a single allocation among many
    // different objects by using the *offset* parameters that we've seen in
    // many functions.
    VkMemoryRequirements mem_reqs = buffer->get_memory_requirements();
    uint32_t memory_type_index = find_memory_type(mem_reqs.memoryTypeBits, properties);
    auto memory = std::make_unique<DeviceMemory>(*_device, mem_reqs.size, memory_type_index);
    buffer->bind_memory(*memory, 0);
    auto pair = std::make_pair(std::move(buffer), std::move(memory));
    return pair;
}

uint32_t vktest::Application::find_memory_type (uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    // VkPhysicalDeviceMemoryProperties: Has two arrays *memoryTypes* and
    // *memoryHeaps*. Memory heaps are distinct memory resources like dedicated
    // VRAM and swap space in RAM for when VRAM runs out. The different types
    // of memory exist within these heaps.
    VkPhysicalDeviceMemoryProperties memprops = _physical_device->get_memory_properties();
    // Right now we'll only concern ourselves with the type of memory and not
    // the heap it comes from, but you can imagine that this can affect
    // performance.
    for (uint32_t i = 0; i < memprops.memoryTypeCount; i++) {
        if (type_filter & (1 << i)
         && (memprops.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory");
}

void vktest::Application::copy_buffer (const Buffer &src, const Buffer &dest, VkDeviceSize size) const {
    // Memory transfer operations are executed using command buffers, just like
    // drawing commands. Therefore we must first allocate a temporary command
    // buffer.
    //
    // NOTE: You may wish to create a separate command pool for these kinds of
    // short-lived buffers, because the implementation may be able to apply
    // memory allocation optimizations. You should use the
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation
    // in that case.

    CommandBuffer cmdbuf = begin_single_time_commands();
    std::vector<VkBufferCopy> copy_regions { {0, 0, size} };
    cmdbuf.copy_buffer(src, dest, copy_regions);
    end_single_time_commands( std::move(cmdbuf) );
}

vktest::CommandBuffer vktest::Application::begin_single_time_commands () const {
    std::vector<CommandBuffer> cmdbufs = _command_pool->allocate_buffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    CommandBuffer cmdbuf = std::move(cmdbufs[0]);
    cmdbuf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    return cmdbuf;
}

void vktest::Application::end_single_time_commands (CommandBuffer cmdbuf) const {
    cmdbuf.end();

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkCommandBuffer native_cmdbuf = cmdbuf.get_native();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &native_cmdbuf;
    // Command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT
    // synchronization at the beginning.
    _graphics_queue->submit(submit_info, nullptr);
    // There are again two possible ways to wait on this transfer to complete.
    // We could use a fence and wait with *vkWaitForFences*, or simply wait for
    // the transfer queue to become idle with *vkQueueWaitIdle*. A fence would
    // allow you to schedule multiple transfers simultaneously and wait for all
    // of them complete, instead of executing one at a time. That may give the
    // driver more opportunities to optimize.
    _graphics_queue->wait_idle();
}

void vktest::Application::create_command_buffers () {
    _swap_chain->create_command_buffers(*_command_pool);
}

void vktest::Application::record_command_buffers () const {
    std::vector<CommandBuffer*> cmdbufs = _swap_chain->get_command_buffers();
    const std::vector<Framebuffer> &framebufs = _swap_chain->get_framebuffers();

    for (size_t i = 0; i < cmdbufs.size(); i++) {
        const CommandBuffer &cmdbuf = *(cmdbufs[i]);

        cmdbuf.begin();
        VkRect2D render_area { {0, 0}, _swap_chain->get_extent() };
        cmdbuf.begin_render_pass(*_render_pass, framebufs[i], std::move(render_area));
            cmdbuf.bind_pipeline(*_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
            std::vector<VkBuffer> vertex_buffers { _vertex_buffer->get_native() };
            std::vector<VkDeviceSize> offsets { 0 };
            cmdbuf.bind_vertex_buffers(0, 1, vertex_buffers, offsets);
            cmdbuf.bind_index_buffer(*_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            VkDescriptorSet descriptor_set = _descriptor_sets[i].get_native();
            vkCmdBindDescriptorSets(cmdbuf.get_native(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    _pipeline_layout->get_native(),
                                    0,
                                    1, &descriptor_set,
                                    0, 0);

            cmdbuf.draw_indexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        cmdbuf.end_render_pass();
        cmdbuf.end();
    }
}

void vktest::Application::create_sync_objects () {
    _image_available_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _render_finished_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _in_flight_fences.reserve(MAX_FRAMES_IN_FLIGHT);
    _images_in_flight.resize(_swap_chain->get_images().size(), nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _image_available_semaphores.emplace_back(*_device);
        _render_finished_semaphores.emplace_back(*_device);
        _in_flight_fences.emplace_back(*_device, VK_FENCE_CREATE_SIGNALED_BIT);
    }
}

void vktest::Application::draw () {
    _in_flight_fences[_current_frame].wait(UINT64_MAX);

    std::optional<uint32_t> image_index = acquire_image();
    if (!image_index) {
        recreate_swap_chain();
        return;
    }
    if (_images_in_flight[*image_index] != nullptr) {
        _images_in_flight[*image_index]->wait(UINT64_MAX);
    }
    _images_in_flight[*image_index] = &_in_flight_fences[_current_frame];
    _in_flight_fences[_current_frame].reset();

    update_uniform_buffer(*image_index);
    submit_command_buffer(*image_index);
    present(*image_index);
    _current_frame = (_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::optional<uint32_t> vktest::Application::acquire_image () const {
    return _swap_chain->acquire_next_image(UINT64_MAX, &_image_available_semaphores[_current_frame], nullptr);
}

void vktest::Application::update_uniform_buffer (uint32_t image_index) const {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,std::chrono::seconds::period>(current_time - start_time).count();

    UniformBufferObject ubo {};
    // existing transformation (identity matrix here), angle, axis
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // eye position, center position, up axis
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // vertical fov, aspect ratio, near, far
    ubo.proj = glm::perspective(glm::radians(45.0f), _swap_chain->get_extent().width / (float) _swap_chain->get_extent().height, 0.1f, 10.0f);
    // GLM was originally designed for OpenGL, where the Y coordinate of the
    // clip coordinates is inverted.
    ubo.proj[1][1] *= -1;

    // NOTE: Using a UBO this way is not the most efficient way to pass
    // frequently changing values to the shader. A more efficient way to pass
    // a small buffer of data to shaders are *push constants*.
    size_t idx = static_cast<size_t>(image_index);
    void *data = _uniform_buffer_memories[idx]->map(0, sizeof(ubo));
    std::memcpy(data, &ubo, sizeof(ubo));
    _uniform_buffer_memories[idx]->unmap();
}

void vktest::Application::submit_command_buffer (uint32_t image_index) const {
    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { _image_available_semaphores[_current_frame].get_native() };
    // We want to wait with writing colors to the image until it's available.
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    // Semaphores to wait on before execution begins.
    submit_info.pWaitSemaphores = wait_semaphores;
    // Stage(s) of the pipeline to wait. Each entry in the waitStages array
    // corresponds to the semaphore with the same index in pWaitSemaphores.
    submit_info.pWaitDstStageMask = wait_stages;

    // Command buffers to actually submit for execution.
    submit_info.commandBufferCount = 1;
    VkCommandBuffer cmdbuf = _swap_chain->get_command_buffer(image_index).get_native();
    submit_info.pCommandBuffers = &cmdbuf;

    // Semaphores to signal once the command buffer(s) have finished execution.
    VkSemaphore signal_semaphores[] = { _render_finished_semaphores[_current_frame].get_native() };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    _graphics_queue->submit(submit_info, &_in_flight_fences[_current_frame]);
}

void vktest::Application::present (uint32_t image_index) {
    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    VkSemaphore signal_semaphores[] = { _render_finished_semaphores[_current_frame].get_native() };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { _swap_chain->get_native() };
    // The swap chains to present images to and the index of the image for each
    // swap chain.
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    // It allows you to specify an array of VkResult values to check for every
    // individual swap chain if presentation was successful.
    present_info.pResults = nullptr; // Optional
    bool res = _present_queue->present(present_info);
    if (!res || _framebuffer_resized) {
        _framebuffer_resized = false;
        recreate_swap_chain();
    }
}

// Recreates swap chain and all of the objects that depend on the swap chain or
// the window size.
void vktest::Application::recreate_swap_chain () {
    handle_minimization();
    _device->wait_idle();
    cleanup_swap_chain();
    create_swap_chain();
    create_render_pass();
    // NOTE: It is possible to avoid pipeline recreation by using dynamic state
    // for the viewports and scissor rectangles.
    create_pipeline();
    create_color_resources();
    create_depth_resources();
    create_framebuffers();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
    record_command_buffers();
}

void vktest::Application::cleanup_swap_chain () {
    _pipeline.reset();
    _pipeline_layout.reset();
    _render_pass.reset();
    _command_pool->free_buffers( _swap_chain->get_command_buffers() );
    _swap_chain.reset();
    _uniform_buffers.clear();
    _uniform_buffer_memories.clear();
    _descriptor_sets.clear();
    _descriptor_pool.reset();
}

void vktest::Application::handle_minimization () const noexcept {
    // Pauses when minimized
    int width = 0, height = 0;
    _window->get_framebuffer_size(&width, &height);
    while (width == 0 || height == 0) {
        _window->get_framebuffer_size(&width, &height);
        glfwWaitEvents();
    }
}
