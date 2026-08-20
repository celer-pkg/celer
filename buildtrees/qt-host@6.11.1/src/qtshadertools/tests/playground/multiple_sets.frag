#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D tex;
layout(std140, binding = 1) uniform buf {
    vec4 color;
};

layout(set = 1, binding = 0) uniform sampler2D tex2;
layout(std140, set = 1, binding = 1) uniform buf2 {
    vec4 color2;
};
layout(set = 1, binding = 2) uniform sampler2D tex3;

void main()
{
    fragColor = texture(tex, v_texcoord) * texture(tex2, v_texcoord) * color * color2 * texture(tex3, v_texcoord);
}
