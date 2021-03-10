#include "SwapChainSupport.hpp"

bool vktest::SwapChainSupport::is_adequate () const noexcept {
    return !formats.empty() && !present_modes.empty();
}
