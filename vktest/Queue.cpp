#include "Queue.hpp"
#include <stdexcept>

VkQueue vktest::Queue::get_native () const noexcept {
    return _native;
}

void vktest::Queue::submit (const VkSubmitInfo &submit_info, const Fence *fence) const {
    VkResult res = vkQueueSubmit(_native, 1, &submit_info, fence ? fence->get_native() : VK_NULL_HANDLE);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to submit comman buffer");
}

void vktest::Queue::submit (const std::vector<VkSubmitInfo> &submit_infos, const Fence *fence) const {
    VkResult res = vkQueueSubmit(_native, static_cast<uint32_t>(submit_infos.size()), submit_infos.data(), fence ? fence->get_native() : VK_NULL_HANDLE);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to submit comman buffer");
}

bool vktest::Queue::present (const VkPresentInfoKHR &present_info) const {
    VkResult res = vkQueuePresentKHR(_native, &present_info);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        return false;
    } else if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }
    return true;
}

void vktest::Queue::wait_idle () const noexcept {
    vkQueueWaitIdle(_native);
}

vktest::Queue::Queue (const Device &device, uint32_t queue_family_index, uint32_t queue_index) noexcept {
    vkGetDeviceQueue(
        device.get_native(),
        queue_family_index,
        queue_index,
        &_native
    );
}
