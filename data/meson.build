shader_sources = files(
    'shader.frag',
    'shader.vert'
)

fs = import('fs')
glslang = find_program('glslangValidator')

shaders = []
foreach src : shader_sources
    name = fs.name( '@0@.spv'.format(src) )
    shaders += custom_target(name,
        input: src, output: name,
        command: [ glslang, '-V', '@INPUT@', '-o', '@OUTPUT@' ])
endforeach

assets = files(
    'model.obj',
    'texture.png'
)

foreach asset : assets
    name = fs.name( '@0@'.format(asset) )
    configure_file(
        input: asset,
        output: name,
        copy: true)
endforeach
