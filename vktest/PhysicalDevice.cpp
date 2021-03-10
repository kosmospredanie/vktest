#include "PhysicalDevice.hpp"
#include "config.hpp"
#include <map>
#include <vector>
#include <stdexcept>
#include <set>

namespace vktest {
    static QueueFamilyIndices find_queue_families (VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> families {count};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

        QueueFamilyIndices indices;
        uint32_t i = 0;
        for (VkQueueFamilyProperties family : families) {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics = i;
            else if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transfer = i;
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
            if (present_support) indices.present = i;
            if (indices.is_complete()) break;
            i++;
        }
        return indices;
    }

    static bool check_device_extensions (VkPhysicalDevice device) noexcept {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> availables {count};
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availables.data());
        std::set<std::string> required_extensions { device_extensions.begin(), device_extensions.end() };
        for (VkExtensionProperties ext : availables) {
            required_extensions.erase(ext.extensionName);
        }
        return required_extensions.empty();
    }

    static SwapChainSupport query_swap_chain_support (VkPhysicalDevice device, const Surface &surface) noexcept {
        SwapChainSupport support;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.get_native(), &support.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get_native(), &format_count, nullptr);
        if (format_count > 0) {
            support.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get_native(), &format_count, support.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get_native(), &present_mode_count, nullptr);
        if (present_mode_count > 0) {
            support.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get_native(), &present_mode_count, support.present_modes.data());
        }
        return support;
    }

    static int32_t rate_device (VkPhysicalDevice device, const Surface &surface) {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);

        if (!features.geometryShader || !features.samplerAnisotropy) return 0;
        QueueFamilyIndices indices = find_queue_families(device, surface.get_native());
        if (!indices.is_complete()) return 0;
        if (!check_device_extensions(device)) return 0;
        SwapChainSupport swap_chain_support = query_swap_chain_support(device, surface);
        if (!swap_chain_support.is_adequate()) return 0;

        int32_t score = 0;
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        score += properties.limits.maxImageDimension2D; // Maximum size of textures
        return score;
    }

    static VkPhysicalDevice select_device (const std::vector<VkPhysicalDevice> &devices, const Surface &surface) {
        std::multimap<int32_t,VkPhysicalDevice> candidates;
        for (VkPhysicalDevice device : devices) {
            int32_t score = rate_device(device, surface);
            candidates.insert( std::make_pair(score, device) );
        }
        auto selected = candidates.rbegin();
        if (selected->first == 0) throw std::runtime_error("Failed to find a suitable physical device");
        return selected->second;
    }
}

VkPhysicalDevice vktest::PhysicalDevice::get_native () const noexcept {
    return _native;
}

const vktest::QueueFamilyIndices &vktest::PhysicalDevice::get_queue_families () const noexcept {
    return _queue_families;
}

vktest::SwapChainSupport vktest::PhysicalDevice::query_swap_chain_support (const Surface &surface) const noexcept {
    return vktest::query_swap_chain_support(_native, surface);
}

VkPhysicalDeviceMemoryProperties vktest::PhysicalDevice::get_memory_properties () const noexcept {
    VkPhysicalDeviceMemoryProperties memprops {};
    vkGetPhysicalDeviceMemoryProperties(_native, &memprops);
    return memprops;
}

VkPhysicalDeviceProperties vktest::PhysicalDevice::get_properties () const noexcept {
    VkPhysicalDeviceProperties props {};
    vkGetPhysicalDeviceProperties(_native, &props);
    return props;
}

VkFormatProperties vktest::PhysicalDevice::get_format_properties (VkFormat format) const noexcept {
    VkFormatProperties props {};
    vkGetPhysicalDeviceFormatProperties(_native, format, &props);
    return props;
}

std::unique_ptr<vktest::PhysicalDevice> vktest::PhysicalDevice::select (const Instance &instance, const Surface &surface) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance.get_native(), &count, nullptr);
    if (count == 0) throw std::runtime_error("Failed to find physical devices with Vulkan support");
    std::vector<VkPhysicalDevice> devices {count};
    vkEnumeratePhysicalDevices(instance.get_native(), &count, devices.data());

    VkPhysicalDevice native = select_device(devices, surface);
    return std::unique_ptr<PhysicalDevice> { new PhysicalDevice(native, surface) };
}

vktest::PhysicalDevice::PhysicalDevice (VkPhysicalDevice native, const Surface &surface) noexcept {
    _native = native;
    _queue_families = find_queue_families(_native, surface.get_native());
}
