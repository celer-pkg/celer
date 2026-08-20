#version 440

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;

layout(std140, binding = 0) uniform buf {
    vec2 blend;
    vec2 padding;
} ubuf;

layout(binding = 1) uniform sampler2D tex1;
layout(binding = 2) uniform sampler2D tex2;

void main()
{
#if QSHADER_HLSL || QSHADER_MSL
    vec2 uv = vec2(uv_coord.x, 1.0 - uv_coord.y);
    vec4 t1 = texture(tex1, uv);
    vec4 t2 = texture(tex2, uv);
#else
    vec4 t1 = texture(tex1, uv_coord);
    vec4 t2 = texture(tex2, uv_coord);
#endif
    fragOutput = t1 * ubuf.blend.x + t2 * ubuf.blend.y;
}
