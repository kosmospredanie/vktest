#ifndef __VKTEST_APPLICATION_HPP__
#define __VKTEST_APPLICATION_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include "Initalization.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Queue.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "CommandPool.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
#include "Buffer.hpp"
#include "DeviceMemory.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"
#include "Vertex.hpp"

namespace vktest {
    class Application {
    public:
        Application (std::string app_name);

    private:
        void init ();
        void loop ();
        void update ();

        void init_window ();
        static void on_framebuffer_resize (GLFWwindow *window, int width, int height);

        void init_vulkan ();
        void create_swap_chain ();
        void create_render_pass ();
        std::vector<VkSubpassDependency> prepare_subpass_dependencies () const noexcept;
        void create_descriptor_set_layout ();
        void create_pipeline ();
        void create_framebuffers ();

        /*
         * For practical applications it is recommended to combine the submit
         * commands in a single command buffer and execute them asynchronously
         * for higher throughput, especially the transitions and copy in the
         * *create_texture_image*.
        */

        void create_color_resources ();
        void create_depth_resources ();
        VkFormat find_depth_format () const;
        bool has_stencil_component (VkFormat format) const noexcept;
        VkFormat find_supported_format (const std::vector<VkFormat>& candidates,
                                        VkImageTiling tiling,
                                        VkFormatFeatureFlags features) const;
        void create_texture_image ();
        std::pair<std::unique_ptr<Image>,std::unique_ptr<DeviceMemory>> create_image (
                uint32_t width,
                uint32_t height,
                uint32_t mip_levels,
                VkSampleCountFlagBits num_samples,
                VkFormat format,
                VkImageTiling tiling,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties) const;
        void generate_mipmaps (const Image &image,
                               VkFormat image_format,
                               int32_t width,
                               int32_t height,
                               uint32_t mip_levels) const;
        void transition_image_layout (
                const Image &image,
                VkFormat format,
                VkImageLayout old_layout,
                VkImageLayout new_layout,
                uint32_t mip_levels) const;
        void copy_buffer_to_image (
                const Buffer &buffer,
                const Image &image,
                uint32_t width,
                uint32_t height) const;
        VkSampleCountFlagBits get_max_usable_sample_count () const noexcept;
        void create_texture_image_view ();
        void create_texture_sampler ();

        void load_model ();
        void create_vertex_buffer ();
        void create_index_buffer ();
        void create_uniform_buffers ();
        std::pair<std::unique_ptr<Buffer>,std::unique_ptr<DeviceMemory>> create_buffer (
                VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
        uint32_t find_memory_type (uint32_t type_filter, VkMemoryPropertyFlags properties) const;
        void copy_buffer (const Buffer &src, const Buffer &dest, VkDeviceSize size) const;

        CommandBuffer begin_single_time_commands () const;
        void end_single_time_commands (CommandBuffer cmdbuf) const;

        void create_descriptor_pool ();
        void create_descriptor_sets ();

        void create_command_buffers ();
        void record_command_buffers () const;
        void create_sync_objects ();

        void draw ();
        std::optional<uint32_t> acquire_image () const;
        void update_uniform_buffer (uint32_t image_index) const;
        void submit_command_buffer (uint32_t image_index) const;
        void present (uint32_t image_index);

        void recreate_swap_chain ();
        void cleanup_swap_chain ();
        void handle_minimization () const noexcept;

        std::string _app_name;
        std::unique_ptr<Initalization> _init;
        std::unique_ptr<Window> _window;

        std::unique_ptr<Instance> _instance;
        const PhysicalDevice *_physical_device;
        VkSampleCountFlagBits _msaa_samples = VK_SAMPLE_COUNT_1_BIT;
        std::unique_ptr<Surface> _surface;
        std::unique_ptr<Device> _device;
        const Queue *_graphics_queue;
        const Queue *_present_queue;
        std::unique_ptr<CommandPool> _command_pool;
        std::unique_ptr<SwapChain> _swap_chain;

        std::unique_ptr<Shader> _vert_shader;
        std::unique_ptr<Shader> _frag_shader;

        std::unique_ptr<RenderPass> _render_pass;
        std::unique_ptr<DescriptorSetLayout> _descriptor_set_layout;
        std::unique_ptr<PipelineLayout> _pipeline_layout;
        std::unique_ptr<Pipeline> _pipeline;

        std::unique_ptr<DeviceMemory> _color_image_memory;
        std::unique_ptr<Image> _color_image;
        std::unique_ptr<ImageView> _color_image_view;

        std::unique_ptr<DeviceMemory> _dpeth_image_memory;
        std::unique_ptr<Image> _depth_image;
        std::unique_ptr<ImageView> _depth_image_view;

        uint32_t _mip_levels;
        std::unique_ptr<DeviceMemory> _texture_image_memory;
        std::unique_ptr<Image> _texture_image;
        std::unique_ptr<ImageView> _texture_image_view;
        std::unique_ptr<Sampler> _texture_sampler;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::unique_ptr<DeviceMemory> _vertex_buffer_memory;
        std::unique_ptr<Buffer> _vertex_buffer;
        std::unique_ptr<DeviceMemory> _index_buffer_memory;
        std::unique_ptr<Buffer> _index_buffer;

        // Uniform buffer per swap chaing image
        std::vector<std::unique_ptr<DeviceMemory>> _uniform_buffer_memories;
        std::vector<std::unique_ptr<Buffer>> _uniform_buffers;
        std::unique_ptr<DescriptorPool> _descriptor_pool;
        std::vector<DescriptorSet> _descriptor_sets;

        // Each frame should have its own set of semaphores.
        std::vector<Semaphore> _image_available_semaphores;
        std::vector<Semaphore> _render_finished_semaphores;
        std::vector<Fence> _in_flight_fences;
        std::vector<const Fence*> _images_in_flight;
        size_t _current_frame;
        bool _framebuffer_resized;
    };
}

#endif /* __VKTEST_APPLICATION_HPP__ */
