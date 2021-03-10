#ifndef __VKTEST_VERTEX_HPP__
#define __VKTEST_VERTEX_HPP__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <functional>
#include <vector>

namespace vktest {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 tex_coord;

        bool operator== (const Vertex &other) const {
            return pos == other.pos && color == other.color && tex_coord == other.tex_coord;
        }

        static std::vector<VkVertexInputBindingDescription> get_binding_descs () {
            std::vector<VkVertexInputBindingDescription> binding_descs (1);
            // Specifies the index of the binding in the array of bindings.
            binding_descs[0].binding = 0;
            // Specifies the number of bytes from one entry to the next.
            binding_descs[0].stride = sizeof(Vertex);
            // The inputRate parameter:
            //  * VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex.
            //  * VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance.
            binding_descs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_descs;
        }

        static std::vector<VkVertexInputAttributeDescription> get_attribute_descs () {
            // An attribute description struct describes how to extract a
            // vertex attribute from a chunk of vertex data originating from a
            // binding description.
            std::vector<VkVertexInputAttributeDescription> attribute_descs (3);

            // binding: Tells Vulkan from which binding the per-vertex data comes.
            attribute_descs[0].binding = 0;
            // location: References the location directive of the
            // input in the vertex shader.
            attribute_descs[0].location = 0;
            // format: Describes the type of data for the attribute
            //  * float: VK_FORMAT_R32_SFLOAT
            //  * vec2: VK_FORMAT_R32G32_SFLOAT
            //  * vec3: VK_FORMAT_R32G32B32_SFLOAT
            //  * vec4: VK_FORMAT_R32G32B32A32_SFLOAT
            //  * ivec2: VK_FORMAT_R32G32_SINT
            //  * uvec4: VK_FORMAT_R32G32B32A32_UINT
            //  * double: VK_FORMAT_R64_SFLOAT
            // It is allowed to use more channels than the number of components
            // in the shader, but they will be silently discarded. If the
            // number of channels is lower than the number of components, then
            // the BGA components will use default values of (0, 0, 1).
            attribute_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descs[0].offset = offsetof(Vertex, pos);

            attribute_descs[1].binding = 0;
            attribute_descs[1].location = 1;
            attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descs[1].offset = offsetof(Vertex, color);

            attribute_descs[2].binding = 0;
            attribute_descs[2].location = 2;
            attribute_descs[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descs[2].offset = offsetof(Vertex, tex_coord);
            return attribute_descs;
        }
    };
}

namespace std {
    template<> struct hash<vktest::Vertex> {
        size_t operator() (const vktest::Vertex &s) const {
            return (  (hash<glm::vec3>()(s.pos)
                    ^ (hash<glm::vec3>()(s.color) << 1)) >> 1)
                    ^ (hash<glm::vec2>()(s.tex_coord) << 1 );
        }
    };
}

#endif /* __VKTEST_VERTEX_HPP__ */
