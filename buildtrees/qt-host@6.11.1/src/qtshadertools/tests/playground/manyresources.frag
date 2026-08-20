#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D tex2;
layout(binding = 3) uniform sampler2D tex3;

layout (binding = 4, rgba8) uniform readonly image2D image0;
layout (binding = 5, rgba8) uniform writeonly image2D image1;

layout(std140, binding = 10) buffer storageBuf {
    vec4 whatever;
} buf;

layout(std140, binding = 11) uniform buf {
    mat4 mvp;
} ubuf;

layout(std140, binding = 12) uniform buf2 {
    float f;
} ubuf2;

void main()
{
    vec4 c = texture(tex, v_texcoord) * texture(tex2, v_texcoord) * texture(tex3, v_texcoord);
    c = ubuf.mvp * c;
    c *= ubuf2.f;
    c *= buf.whatever;
    c += imageLoad(image0, ivec2(0, 0));
    imageStore(image1, ivec2(0, 0), c);
    fragColor = c;
}
