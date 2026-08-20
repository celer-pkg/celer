#version 440

layout(std140, binding = 0) uniform buf1 {
    vec3 v;
    float f;
} ubuf1;

layout(std140, binding = 1) uniform buf2 {
    float f2;
}; // no instance name

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(ubuf1.v, ubuf1.f + f2);
}
