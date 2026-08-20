#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 v_color;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    float opacity;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
// ensure that the manually defined macro still works
#ifdef MY_MACRO
#if MY_VALUE == 321
    v_color = color;
#else
    broken MY_VALUE
#endif
#else
    broken MY_MACRO
#endif

// fail in non-per-target mode
#if !defined(QSHADER_SPIRV) && !defined(QSHADER_GLSL) && !defined(QSHADER_HLSL) && !defined(QSHADER_MSL)
    macros not present?!
#endif

    gl_Position = ubuf.mvp * position;
}
