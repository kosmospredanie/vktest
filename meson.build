project('vktest', 'cpp',
    version: '0.1.0',
    license: 'GPL3',
    meson_version: '>=0.53.0',
    default_options: ['cpp_std=c++17'])

project_desc = 'An example Vulkan project'

message('vktest version: ' + meson.project_version())

cppc = meson.get_compiler('cpp')

m_dep = cppc.find_library('m', required: false)
vulkan_dep = cppc.find_library('vulkan', required: false)
glfw_reqs = '3.3.2'
glfw_dep = dependency('glfw3', version: '>=' + glfw_reqs)
glm_reqs = '0.9.9'
glm_dep = dependency('glm', version: '>=' + glm_reqs)

stb_inc = include_directories('stb')
tinyobjloader_inc = include_directories('tinyobjloader')

dependencies = [
    m_dep,
    vulkan_dep,
    glfw_dep,
    glm_dep
]

incdirs = [
    stb_inc,
    tinyobjloader_inc
]

subdir('data')
subdir('vktest')
