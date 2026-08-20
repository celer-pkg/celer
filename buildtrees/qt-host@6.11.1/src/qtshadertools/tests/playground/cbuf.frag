#version 440

layout(location = 0) in vec3 vECVertNormal;
layout(location = 1) in vec3 vECVertPos;

layout(std140, binding = 1) uniform buf {
    vec3 v;
    float f;
} ubuf;

layout(std140, binding = 2) uniform buf2 {
    float f;
    vec3 v;
} ubuf2;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(ubuf.v, ubuf.f);
}
