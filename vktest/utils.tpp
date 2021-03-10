#include <algorithm>

template <typename T>
std::vector<T*> vktest::pvec (std::vector<T> &vec) {
    std::vector<T*> result (vec.size());
    std::transform(vec.begin(), vec.end(), result.begin(), [](T &obj) { return &obj; });
    return result;
}
