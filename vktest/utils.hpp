#ifndef __VKTEST_UTILS_HPP__
#define __VKTEST_UTILS_HPP__

#include <vector>

namespace vktest {
    template <typename T>
    std::vector<T*> pvec (std::vector<T> &vec);
}

#include "utils.tpp"

#endif /* __VKTEST_UTILS_HPP__ */
