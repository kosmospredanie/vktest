#include "Pipeline.hpp"
#include "Vertex.hpp"
#include <optional>
#include <stdexcept>

namespace vktest {
    static VkPipelineVertexInputStateCreateInfo prepare_vertex_input_info (
            const std::vector<VkVertexInputBindingDescription> &binding_descs,
            const std::vector<VkVertexInputAttributeDescription> &attrib_descs) noexcept {
        VkPipelineVertexInputStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descs.size());
        create_info.pVertexBindingDescriptions = binding_descs.data(); // Optional
        create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrib_descs.size());
        create_info.pVertexAttributeDescriptions = attrib_descs.data(); // Optional
        return create_info;
    }

    static VkPipelineInputAssemblyStateCreateInfo prepare_input_assembly_info () noexcept {
        VkPipelineInputAssemblyStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, etc.
        create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // If you set the primitiveRestartEnable member to VK_TRUE, then it's
        // possible to break up lines and triangles in the _STRIP topology
        // modes by using a special index of 0xFFFF or 0xFFFFFFFF.
        create_info.primitiveRestartEnable = VK_FALSE;
        return create_info;
    }

    static VkPipelineViewportStateCreateInfo prepare_viewport_info (
                const VkViewport &viewport, const VkRect2D &scissor) noexcept {
        VkPipelineViewportStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        // It is possible to use multiple viewports and scissor rectangles on
        // some graphics cards. Using multiple requires enabling a GPU feature
        // (see logical device creation).
        create_info.viewportCount = 1;
        create_info.pViewports = &viewport;
        create_info.scissorCount = 1;
        create_info.pScissors = &scissor;
        return create_info;
    }

    static VkPipelineRasterizationStateCreateInfo prepare_rasterizer_info () noexcept {
        VkPipelineRasterizationStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // If depthClampEnable is set to VK_TRUE, then fragments that are
        // beyond the near and far planes are clamped to them as opposed to
        // discarding them. This is useful in some special cases like shadow
        // maps. Using this requires enabling a GPU feature.
        create_info.depthClampEnable = VK_FALSE;
        // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never
        // passes through the rasterizer stage. This basically disables any
        // output to the framebuffer.
        create_info.rasterizerDiscardEnable = VK_FALSE;

        // VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, or VK_POLYGON_MODE_POINT.
        // Using any mode other than fill requires enabling a GPU feature.
        create_info.polygonMode = VK_POLYGON_MODE_FILL;
        // The maximum line width that is supported depends on the hardware and
        // any line thicker than 1.0f requires you to enable the wideLines GPU
        // feature.
        create_info.lineWidth = 1.0f;

        // NOTE: VK_FRONT_FACE_COUNTER_CLOCKWISE because of the Y-flip we did
        // in the projection matrix.
        create_info.cullMode = VK_CULL_MODE_BACK_BIT; // face culling mode
        create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // the vertex order for faces

        // The rasterizer can alter the depth values by adding a constant value
        // or biasing them based on a fragment's slope. This is sometimes used
        // for shadow mapping.
        create_info.depthBiasEnable = VK_FALSE;
        create_info.depthBiasConstantFactor = 0.0f; // Optional
        create_info.depthBiasClamp = 0.0f; // Optional
        create_info.depthBiasSlopeFactor = 0.0f; // Optional
        return create_info;
    }

    static VkPipelineMultisampleStateCreateInfo prepare_multisample_info (VkSampleCountFlagBits msaa_samples) noexcept {
        // Enabling it requires enabling a GPU feature.
        VkPipelineMultisampleStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        create_info.rasterizationSamples = msaa_samples;
        create_info.sampleShadingEnable = VK_TRUE;
        // Min fraction for sample shading; closer to one is smoother.
        create_info.minSampleShading = 0.2f; // Optional
        create_info.pSampleMask = nullptr; // Optional
        create_info.alphaToCoverageEnable = VK_FALSE; // Optional
        create_info.alphaToOneEnable = VK_FALSE; // Optional
        return create_info;
    }

    static std::optional<VkPipelineDepthStencilStateCreateInfo> prepare_depth_stencil_info () noexcept {
        VkPipelineDepthStencilStateCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        // depthTestEnable: Specifies if the depth of new fragments should be
        // compared to the depth buffer to see if they should be discarded.
        info.depthTestEnable = VK_TRUE;
        // depthWriteEnable: Specifies if the new depth of fragments that pass
        // the depth test should actually be written to the depth buffer.
        info.depthWriteEnable = VK_TRUE;
        // Specifies the comparison that is performed to keep or discard fragments.
        info.depthCompareOp = VK_COMPARE_OP_LESS;

        // Optional depth bound test: this allows you to only keep fragments
        // that fall within the specified depth range.
        info.depthBoundsTestEnable = VK_FALSE;
        info.minDepthBounds = 0.0f; // Optional
        info.maxDepthBounds = 1.0f; // Optional

        info.stencilTestEnable = VK_FALSE;
        info.front = {}; // Optional
        info.back = {}; // Optional
        return info;
    }

    static VkPipelineColorBlendAttachmentState prepare_color_blend_attachment () noexcept {
        // There are two types of structs to configure color blending. The
        // first struct, VkPipelineColorBlendAttachmentState contains the
        // configuration per attached framebuffer and the second struct,
        // VkPipelineColorBlendStateCreateInfo contains the *global* color
        // blending settings.
        VkPipelineColorBlendAttachmentState state {};
        // colorWriteMask determines which channels are actually passed through.
        state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // If blendEnable is set to VK_FALSE, then the new color from the
        // fragment shader is passed through unmodified. Otherwise, the two
        // mixing operations are performed to compute a new color.
        state.blendEnable = VK_FALSE;
        state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        state.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        state.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        // Alpha blending, where we want the new color to be blended with the
        // old color based on its opacity:
        /*
        state.blendEnable = VK_TRUE;
        state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.colorBlendOp = VK_BLEND_OP_ADD;
        state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        state.alphaBlendOp = VK_BLEND_OP_ADD;
        */
        return state;
    }

    static VkPipelineColorBlendStateCreateInfo prepare_color_blend_info (const VkPipelineColorBlendAttachmentState &color_blend_state) noexcept {
        VkPipelineColorBlendStateCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // If you want to use the second method of blending (bitwise
        // combination), then you should set logicOpEnable to VK_TRUE. The
        // bitwise operation can then be specified in the logicOp field.
        // Note that this will automatically disable the first method
        // (VkPipelineColorBlendAttachmentState).
        create_info.logicOpEnable = VK_FALSE;
        create_info.logicOp = VK_LOGIC_OP_COPY; // Optional
        create_info.attachmentCount = 1;
        create_info.pAttachments = &color_blend_state;
        create_info.blendConstants[0] = 0.0f; // Optional
        create_info.blendConstants[1] = 0.0f; // Optional
        create_info.blendConstants[2] = 0.0f; // Optional
        create_info.blendConstants[3] = 0.0f; // Optional
        return create_info;
    }

    static std::optional<VkPipelineDynamicStateCreateInfo> prepare_dynamic_state_info () noexcept {
        return std::nullopt;
    }
}

vktest::Pipeline::Pipeline (const PipelineLayout &layout,
                            const VkViewport &viewport,
                            const VkRect2D &scissor,
                            const std::vector<VkPipelineShaderStageCreateInfo> &stages,
                            const RenderPass &render_pass,
                            VkSampleCountFlagBits msaa_samples) : _layout {&layout} {
    std::vector<VkVertexInputBindingDescription> binding_descs = Vertex::get_binding_descs();
    std::vector<VkVertexInputAttributeDescription> attrib_descs = Vertex::get_attribute_descs();
    auto vertex_input_info = prepare_vertex_input_info(binding_descs, attrib_descs);
    auto input_assemnly = prepare_input_assembly_info();
    auto viewport_state = prepare_viewport_info(viewport, scissor);
    auto rasterizer = prepare_rasterizer_info();
    auto multisampling = prepare_multisample_info(msaa_samples);
    auto depth_stencil = prepare_depth_stencil_info();
    auto color_blend_attachment = prepare_color_blend_attachment();
    auto color_blending = prepare_color_blend_info(color_blend_attachment);
    auto dynamic_state = prepare_dynamic_state_info();

    VkGraphicsPipelineCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = static_cast<uint32_t>( stages.size() );
    create_info.pStages = stages.data();

    create_info.pVertexInputState = &vertex_input_info;
    create_info.pInputAssemblyState = &input_assemnly;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterizer;
    create_info.pMultisampleState = &multisampling;
    create_info.pDepthStencilState = depth_stencil ? &(*depth_stencil) : nullptr; // Optional
    create_info.pColorBlendState = &color_blending;
    create_info.pDynamicState = dynamic_state ? &(*dynamic_state) : nullptr;  // Optional

    create_info.layout = _layout->get_native();

    create_info.renderPass = render_pass.get_native();
    // The index of the sub-pass where this graphics pipeline will be used.
    create_info.subpass = 0;

    // Vulkan allows you to create a new graphics pipeline by deriving from an
    // existing pipeline. These values are only used if the
    // VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the
    // VkGraphicsPipelineCreateInfo.flags field.
    create_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    create_info.basePipelineIndex = -1; // Optional

    VkResult res = vkCreateGraphicsPipelines(_layout->get_device().get_native(), VK_NULL_HANDLE, 1, &create_info, nullptr, &_native);
    if (res != VK_SUCCESS) std::runtime_error("Failed to create graphics pipeline");
}

vktest::Pipeline::Pipeline (Pipeline &&other) noexcept {
    _native = other._native;
    _layout = other._layout;
    other._native = nullptr;
}

vktest::Pipeline::~Pipeline () {
    if (_native != nullptr) vkDestroyPipeline(_layout->get_device().get_native(), _native, nullptr);
}

VkPipeline vktest::Pipeline::get_native () const noexcept {
    return _native;
}

const vktest::PipelineLayout &vktest::Pipeline::get_layout () const noexcept {
    return *_layout;
}
