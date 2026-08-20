#version 440
#extension GL_EXT_multiview : require

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 v_color;

layout(std140, binding = 0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 mvp[QSHADER_VIEW_COUNT];
#else
    mat4 mvp;
#endif
};

void main()
{
    v_color = color;
#if QSHADER_VIEW_COUNT >= 2
    gl_Position = mvp[gl_ViewIndex] * position;
#else
    gl_Position = mvp * position;
#endif
}
