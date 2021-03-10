#include "QueueFamilyIndices.hpp"

bool vktest::QueueFamilyIndices::is_complete () const noexcept {
    return graphics.has_value() && present.has_value();
}
