#include "Surface.hpp"
#include <stdexcept>

vktest::Surface::Surface (const Instance &instance, const Window &window) :
        _instance(&instance), _window(&window) {
    VkResult res = glfwCreateWindowSurface(instance.get_native(), window.get_native(), nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create window surface");
}

vktest::Surface::Surface (Surface &&other) noexcept {
    _native = other._native;
    _instance = other._instance;
    _window = other._window;
    other._native = nullptr;
}

vktest::Surface::~Surface () {
    if (_native != nullptr) vkDestroySurfaceKHR(_instance->get_native(), _native, nullptr);
}

VkSurfaceKHR vktest::Surface::get_native () const noexcept {
    return _native;
}

const vktest::Window &vktest::Surface::get_window () const noexcept {
    return *_window;
}
