#include "Initalization.hpp"

vktest::Initalization::Initalization () : _moved {false} {
    glfwInit();
}

vktest::Initalization::Initalization (Initalization &&other) noexcept : _moved {false} {
    other._moved = true;
}

vktest::Initalization::~Initalization () {
    if (!_moved) glfwTerminate();
}
