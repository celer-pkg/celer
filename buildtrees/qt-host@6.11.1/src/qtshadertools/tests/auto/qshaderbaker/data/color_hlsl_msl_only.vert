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
#if QSHADER_SPIRV || QSHADER_GLSL || QSHADER_GLSL_ES
    wrong
#endif

#if QSHADER_HLSL || QSHADER_MSL

#if QSHADER_HLSL_VERSION == 50 || QSHADER_MSL_VERSION == 12
    v_color = color;
    gl_Position = ubuf.mvp * position;
#else
    version macros broken
#endif

#else
    something is broken
#endif
}
