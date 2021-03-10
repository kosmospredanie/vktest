#ifndef __VKTEST_CONFIG_HPP__
#define __VKTEST_CONFIG_HPP__

#include <vector>
#include "Vertex.hpp"

#define WIDTH 800
#define HEIGHT 600

#define MODEL_PATH "data/model.obj"
#define TEXTURE_PATH "data/texture.png"

/**
 * Defines how many frames should be processed concurrently.
 */
#define MAX_FRAMES_IN_FLIGHT 2

namespace vktest {
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

#endif /* __VKTEST_CONFIG_HPP__ */

