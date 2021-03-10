#include "Device.hpp"
#include "config.hpp"
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iterator>

namespace vktest {
    /**
     * Device-only layers are now deprecated, and latest Vulkan no longer
     * distinguishes between instance and device layers. That means that the
     * enabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo are
     * ignored by up-to-date implementations.
     *
     * This function remains to maintain compatibility with implementations
     * released prior to device-layer deprecation.
     *
     * See https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
     */
    static void fill_layers_info (VkDeviceCreateInfo &create_info) noexcept {
        if (validation_layers.empty()) {
            create_info.enabledLayerCount = 0;
        } else {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }
    }
}

vktest::Device::Device (const PhysicalDevice &physical_device,
                        const std::vector<QueueCreateDesc> &queue_create_descs)
        : _physical_device {&physical_device}, _queues {} {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos {};
    for (const QueueCreateDesc &desc : queue_create_descs) {
        VkDeviceQueueCreateInfo queue_create_info {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = desc.family_index;
        queue_create_info.queueCount = desc.count;
        queue_create_info.pQueuePriorities = desc.priorities.data();
        queue_create_infos.push_back( std::move(queue_create_info) );
    }

    // The set of device features that we'll be using. These are the features
    // that we queried support for with vkGetPhysicalDeviceFeatures()
    VkPhysicalDeviceFeatures features {};
    // Anisotropic filtering is an optional device feature, we request it.
    features.samplerAnisotropy = VK_TRUE;
    // Enable sample shading
    features.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &features;
    create_info.enabledExtensionCount = static_cast<uint32_t>( device_extensions.size() );
    create_info.ppEnabledExtensionNames = device_extensions.data();
    fill_layers_info(create_info);

    VkResult res = vkCreateDevice(physical_device.get_native(), &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create logical device");
    fetch_queues(queue_create_descs);
}

vktest::Device::Device (Device &&other) noexcept : _queues {} {
    _native = other._native;
    _physical_device = other._physical_device;
    _queues.insert( std::make_move_iterator(other._queues.begin()),
                    std::make_move_iterator(other._queues.end()) );
    other._native = nullptr;
}

vktest::Device::~Device () {
    if (_native != nullptr) vkDestroyDevice(_native, nullptr);
}

VkDevice vktest::Device::get_native () const noexcept {
    return _native;
}

const vktest::PhysicalDevice &vktest::Device::get_physical_device () const noexcept {
    return *_physical_device;
}

vktest::Queue &vktest::Device::get_queue (uint32_t queue_family_index, uint32_t queue_index) {
    std::pair<uint32_t,uint32_t> key = std::make_pair(queue_family_index, queue_index);
    auto it = _queues.find(key);
    if (it == _queues.end()) {
        throw std::runtime_error("Queue not found");
    } else {
        return *(it->second);
    }
}

void vktest::Device::wait_idle () const noexcept {
    vkDeviceWaitIdle(_native);
}

void vktest::Device::wait_for_fences (const std::vector<const Fence*> fences, bool wait_all, uint64_t timeout) const noexcept {
    std::vector<VkFence> native_fences (fences.size());
    std::transform(fences.begin(), fences.end(), native_fences.begin(), [](const Fence *t) { return t->get_native(); });
    vkWaitForFences(_native, static_cast<uint32_t>(fences.size()), native_fences.data(), wait_all, timeout);
}

void vktest::Device::reset_fences (const std::vector<const Fence*> fences) const noexcept {
    std::vector<VkFence> native_fences (fences.size());
    std::transform(fences.begin(), fences.end(), native_fences.begin(), [](const Fence *t) { return t->get_native(); });
    vkResetFences(_native, static_cast<uint32_t>(fences.size()), native_fences.data());
}

void vktest::Device::fetch_queues (const std::vector<QueueCreateDesc> &queue_create_descs) noexcept {
    for (const QueueCreateDesc &desc : queue_create_descs) {
        for (uint32_t i = 0; i < desc.count; i++) {
            std::pair<uint32_t,uint32_t> key = std::make_pair(desc.family_index, i);
            std::unique_ptr<Queue> queue = std::unique_ptr<Queue>( new Queue {*this, desc.family_index, i} );
            _queues.emplace(key, std::move(queue));
        }
    }
}
