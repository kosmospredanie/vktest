#include "Window.hpp"

vktest::Window::Window (int width, int height, const std::string &name) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _native = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}

vktest::Window::Window (Window &&other) noexcept {
    _native = other._native;
    other._native = nullptr;
}

vktest::Window::~Window () {
    if (_native != nullptr) glfwDestroyWindow(_native);
}

GLFWwindow *vktest::Window::get_native () const noexcept {
    return _native;
}

void *vktest::Window::get_user_pointer () const noexcept {
    return glfwGetWindowUserPointer(_native);
}

void vktest::Window::set_user_pointer (void *pointer) const noexcept {
    glfwSetWindowUserPointer(_native, pointer);
}

void vktest::Window::set_framebuffer_resize_callback (GLFWframebuffersizefun callback) const noexcept {
    glfwSetFramebufferSizeCallback(_native, callback);
}

void vktest::Window::get_framebuffer_size (int *width, int *height) const noexcept {
    glfwGetFramebufferSize(_native, width, height);
}
