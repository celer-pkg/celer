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
#if QSHADER_SPIRV || QSHADER_HLSL || QSHADER_MSL
    wrong
#endif

#if QSHADER_GLSL && !QSHADER_GLSL_ES && (QSHADER_GLSL_VERSION == 330 || QSHADER_GLSL_VERSION == 440)
    v_color = color;
    gl_Position = ubuf.mvp * position;
#else
    something is broken
#endif
}
