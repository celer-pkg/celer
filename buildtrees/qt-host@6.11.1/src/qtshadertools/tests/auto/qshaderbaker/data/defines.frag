#version 440

layout(location = 0) in vec3 v_color;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
#if OPACITY_SIZE == 1
    float opacity;
#elif OPACITY_SIZE == 2
    vec2 opacity;
#elif OPACITY_SIZE == 3
    vec3 opacity;
#else
    vec4 opacity;
#endif
} ubuf;

#ifdef DO_NOT_BREAK
void main()
#endif
{
#if OPACITY_SIZE == 1
    fragColor = vec4(v_color * ubuf.opacity, ubuf.opacity);
#else
    fragColor = vec4(v_color * ubuf.opacity.r, ubuf.opacity.r);
#endif
}
