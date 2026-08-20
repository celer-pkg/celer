#version 440
#extension GL_EXT_multiview : require

layout(location = 0) in vec4 pos;

layout(std140, binding = 0) uniform buf
{
    mat4 mvp[2];
};

void main()
{
    gl_Position = mvp[gl_ViewIndex] * pos;
}
