#ifndef __VKTEST_WINDOW_HPP__
#define __VKTEST_WINDOW_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vktest {
    class Window {
    public:
        Window (int width, int height, const std::string &name);
        Window (const Window &) = delete;
        Window (Window &&other) noexcept;
        ~Window ();
        GLFWwindow *get_native () const noexcept;
        void *get_user_pointer () const noexcept;
        void set_user_pointer (void *pointer) const noexcept;
        void set_framebuffer_resize_callback (GLFWframebuffersizefun callback) const noexcept;
        void get_framebuffer_size (int *width, int *height) const noexcept;

    private:
        GLFWwindow *_native;
    };
}

#endif /* __VKTEST_WINDOW_HPP__ */
