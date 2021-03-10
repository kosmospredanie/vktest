#include "Instance.hpp"
#include "config.hpp"
#include <stdexcept>
#include <cstring>
#include <vector>

namespace vktest {
    static bool check_validation_layers () noexcept {
        if (validation_layers.empty()) return true;
        uint32_t available;
        vkEnumerateInstanceLayerProperties(&available, nullptr);
        std::vector<VkLayerProperties> available_layers{available};
        vkEnumerateInstanceLayerProperties(&available, available_layers.data());

        for (const char *name : vktest::validation_layers) {
            bool found = false;
            for (const VkLayerProperties &prop : available_layers) {
                if (strcmp(name, prop.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    static std::vector<const char*> get_required_extensions () noexcept {
        uint32_t num_glfw_extensions = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions); // VK_KHR_surface, etc.
        std::vector<const char*> extensions {glfw_extensions, glfw_extensions + num_glfw_extensions};
        extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        return extensions;
    }
}

vktest::Instance::Instance (const std::string &app_name, uint32_t api_version) {
    if (!check_validation_layers()) {
        throw std::runtime_error("Validation layers not available");
    }

    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = api_version;

    // This requires *VK_EXT_validation_features* extension
    VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = &features;
    create_info.pApplicationInfo = &app_info;

    std::vector<const char*> extensions = get_required_extensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    if (validation_layers.empty()) {
        create_info.enabledLayerCount = 0;
    } else {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }

    VkResult res = vkCreateInstance(&create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create instance");
}

vktest::Instance::Instance (Instance &&other) noexcept {
    _native = other._native;
    _physical_device = std::move(other._physical_device);
    other._native = nullptr;
    other._physical_device.reset();
}

vktest::Instance::~Instance () {
    if (_native != nullptr) vkDestroyInstance(_native, nullptr);
}

VkInstance vktest::Instance::get_native () const noexcept {
    return _native;
}

vktest::PhysicalDevice &vktest::Instance::get_physical_device () const noexcept {
    return *_physical_device;
}

void vktest::Instance::select_physical_device (const Surface &surface) {
    _physical_device = PhysicalDevice::select(*this, surface);
}
