#ifndef __VKTEST_UNIFORMBUFFEROBJECT_HPP__
#define __VKTEST_UNIFORMBUFFEROBJECT_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace vktest {
    /**
     * @see https://en.cppreference.com/w/cpp/language/alignas
     * @see GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
     */
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
}

#endif /* __VKTEST_UNIFORMBUFFEROBJECT_HPP__ */
